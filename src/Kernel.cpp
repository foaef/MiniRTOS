#include "Kernel.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

namespace ebike {

Task::Task(TaskSpec spec, std::function<void()> body)
    : spec_(std::move(spec)), body_(std::move(body)), effectivePriority_(spec_.priority()) {
    if (!body_) {
        throw ConfigError("task '" + spec_.name() + "' has no body");
    }
}

void Task::startJob(Tick now) {
    state_ = TaskState::Ready;
    jobRelease_ = now;
    jobDeadline_ = now + spec_.deadline();
    remaining_ = spec_.wcet();
    missedThisJob_ = false;
    nextRelease_ += spec_.period();
    ++jobs_;
}

void Task::skipRelease() {
    nextRelease_ += spec_.period();
    ++overruns_;
}

bool Task::checkDeadline(Tick now) {
    if (overdue(now) && !missedThisJob_) {
        missedThisJob_ = true;
        ++misses_;
        return true;
    }
    return false;
}

void Task::runOneTick(Tick now) {
    state_ = TaskState::Running;
    lastRun_ = now;
    --remaining_;
    ++runTicks_;
}

void Task::complete(Tick now) {
    body_();
    maxResponse_ = std::max(maxResponse_, now + 1 - jobRelease_);
    ++completed_;
    state_ = TaskState::Dormant;
    effectivePriority_ = spec_.priority();
}

void Task::preempt() {
    if (state_ == TaskState::Running) {
        state_ = TaskState::Ready;
    }
}

void Task::block() { state_ = TaskState::Blocked; }

void Task::unblock() {
    if (state_ == TaskState::Blocked) {
        state_ = TaskState::Ready;
    }
}

void Task::boostPriority(Priority priority) { effectivePriority_ = std::max(effectivePriority_, priority); }
void Task::restorePriority() { effectivePriority_ = spec_.priority(); }

std::ostream& operator<<(std::ostream& os, const Task& task) {
    return os << std::left << std::setw(16) << task.name() << std::right << std::setw(4)
              << static_cast<int>(task.spec_.priority()) << std::setw(8) << task.jobs_ << std::setw(8)
              << task.completed_ << std::setw(8) << task.misses_ << std::setw(9) << task.overruns_
              << std::setw(9) << task.maxResponse_ << std::setw(8) << (task.usesBus() ? "yes" : "no");
}

Mutex::Mutex(std::string name) : name_(std::move(name)) {}

bool Mutex::tryLock(Task& task) {
    if (owner_ == nullptr || owner_ == &task) {
        owner_ = &task;
        return true;
    }
    if (std::find(waiters_.begin(), waiters_.end(), &task) == waiters_.end()) {
        waiters_.push_back(&task);
        ++contentions_;
    }
    if (task.effectivePriority() > owner_->effectivePriority()) {
        ++inheritances_;
        owner_->boostPriority(task.effectivePriority());
    }
    return false;
}

std::size_t Mutex::unlock(const Task& task) {
    if (owner_ != &task) {
        return 0;
    }
    owner_->restorePriority();
    owner_ = nullptr;
    const std::size_t woken = waiters_.size();
    for (Task* waiter : waiters_) {
        waiter->unblock();
    }
    waiters_.clear();
    return woken;
}

std::ostream& operator<<(std::ostream& os, const Mutex& mutex) {
    return os << "Mutex{" << mutex.name_ << ", owner=" << (mutex.owner_ != nullptr ? mutex.owner_->name() : "-")
              << ", contentions=" << mutex.contentions_ << ", inheritances=" << mutex.inheritances_ << "}";
}

SoftwareTimer::SoftwareTimer(TimerSpec spec, std::function<void()> action)
    : spec_(std::move(spec)), action_(std::move(action)), next_(spec_.period()) {
    if (!action_) {
        throw ConfigError("timer '" + spec_.name() + "' has no action");
    }
}

bool SoftwareTimer::fireIfDue(Tick now) {
    if (now < next_) {
        return false;
    }
    next_ += spec_.period();
    ++fires_;
    action_();
    return true;
}

std::ostream& operator<<(std::ostream& os, const SoftwareTimer& timer) {
    return os << "Timer{" << timer.spec_.name() << ", every " << timer.spec_.period() << ", fired " << timer.fires_
              << "}";
}

void TimerService::add(TimerSpec spec, std::function<void()> action) { timers_.emplace_back(std::move(spec), std::move(action)); }

void TimerService::tick(Tick now) {
    for (SoftwareTimer& timer : timers_) {
        static_cast<void>(timer.fireIfDue(now));
    }
}

std::ostream& operator<<(std::ostream& os, const TimerService& service) {
    os << "TimerService{";
    for (std::size_t i = 0; i < service.timers_.size(); ++i) {
        os << (i == 0 ? "" : ", ") << service.timers_[i];
    }
    return os << "}";
}

Watchdog::Watchdog(Tick timeout) : timeout_(timeout) {
    if (timeout_ == 0) {
        throw ConfigError("watchdog timeout must be positive");
    }
}

void Watchdog::reset(Tick now) {
    ++resets_;
    lastFeed_ = now;
}

std::ostream& operator<<(std::ostream& os, const Watchdog& dog) {
    return os << "Watchdog{timeout=" << dog.timeout_ << ", resets=" << dog.resets_ << "}";
}

Scheduler::Scheduler() : bus_("bus") {}

void Scheduler::addTask(TaskSpec spec, std::function<void()> body) {
    tasks_.push_back(std::make_unique<Task>(std::move(spec), std::move(body)));
}

void Scheduler::releaseJobs(Tick now, TraceBuffer& trace) {
    for (const auto& task : tasks_) {
        if (!task->due(now)) {
            continue;
        }
        if (task->pending()) {
            task->skipRelease();
            trace.record(now, TraceKind::Overrun, task->name());
        } else {
            task->startJob(now);
        }
    }
}

void Scheduler::checkDeadlines(Tick now, TraceBuffer& trace) {
    for (const auto& task : tasks_) {
        if (task->checkDeadline(now)) {
            trace.record(now, TraceKind::DeadlineMiss, task->name());
        }
    }
}

bool Scheduler::moreUrgent(const Task& candidate, const Task& best) {
    if (candidate.effectivePriority() != best.effectivePriority()) {
        return candidate.effectivePriority() > best.effectivePriority();
    }
    return candidate.lastRun() < best.lastRun();
}

Task* Scheduler::pickNext(Tick now, TraceBuffer& trace) {
    for (;;) {
        Task* best = nullptr;
        for (const auto& task : tasks_) {
            if (task->runnable() && (best == nullptr || moreUrgent(*task, *best))) {
                best = task.get();
            }
        }
        if (best == nullptr || !best->usesBus() || bus_.tryLock(*best)) {
            return best;
        }
        best->block();
        trace.record(now, TraceKind::Block, best->name() + " waits for the bus");
    }
}

void Scheduler::finishJob(Task& task, Tick now, TraceBuffer& trace) {
    task.complete(now);
    if (task.usesBus()) {
        const std::size_t woken = bus_.unlock(task);
        if (woken > 0) {
            trace.record(now, TraceKind::Unblock, std::to_string(woken) + " task(s) released by " + task.name());
        }
    }
}

void Scheduler::tick(Tick now, TraceBuffer& trace) {
    ++ticks_;
    releaseJobs(now, trace);
    checkDeadlines(now, trace);
    Task* next = pickNext(now, trace);
    if (next == nullptr) {
        ++idleTicks_;
        running_ = nullptr;
        return;
    }
    if (next != running_) {
        ++switches_;
        trace.record(now, TraceKind::Switch, (running_ != nullptr ? running_->name() : "idle") + " -> " + next->name());
    }
    for (const auto& task : tasks_) {
        if (task.get() != next) {
            task->preempt();
        }
    }
    running_ = next;
    next->runOneTick(now);
    if (next->finished()) {
        finishJob(*next, now, trace);
    }
}

bool Scheduler::hasOverdueJob(Tick now) const {
    return std::any_of(tasks_.begin(), tasks_.end(), [now](const auto& task) { return task->overdue(now); });
}

std::size_t Scheduler::totalDeadlineMisses() const {
    std::size_t total = 0;
    for (const auto& task : tasks_) {
        total += task->misses();
    }
    return total;
}

double Scheduler::cpuUtilization() const {
    return ticks_ == 0 ? 0.0 : static_cast<double>(ticks_ - idleTicks_) / static_cast<double>(ticks_);
}

std::ostream& operator<<(std::ostream& os, const Scheduler& scheduler) {
    return os << "Scheduler{" << scheduler.tasks_.size() << " tasks, CPU " << Format::percent(scheduler.cpuUtilization())
              << ", switches=" << scheduler.switches_ << ", " << scheduler.bus_ << "}";
}

Kernel::Kernel(std::size_t traceCapacity, Tick watchdogTimeout) : watchdog_(watchdogTimeout), trace_(traceCapacity) {}

void Kernel::addTask(TaskSpec spec, std::function<void()> body) { scheduler_.addTask(std::move(spec), std::move(body)); }
void Kernel::addTimer(TimerSpec spec, std::function<void()> action) { timers_.add(std::move(spec), std::move(action)); }

void Kernel::feedWatchdogIfHealthy() {
    if (!scheduler_.hasOverdueJob(now_)) {
        watchdog_.feed(now_);
    }
}

void Kernel::note(TraceKind kind, const std::string& text) { trace_.record(now_, kind, text); }

void Kernel::run(Tick duration, const std::function<void(Tick)>& hardwareTick, const std::function<void()>& onWatchdogReset) {
    const Tick end = now_ + duration;
    while (now_ < end) {
        hardwareTick(now_);
        timers_.tick(now_);
        scheduler_.tick(now_, trace_);
        if (watchdog_.expired(now_)) {
            watchdog_.reset(now_);
            trace_.record(now_, TraceKind::WatchdogReset, "watchdog expired");
            onWatchdogReset();
        }
        ++now_;
    }
}

std::ostream& operator<<(std::ostream& os, const Kernel& kernel) {
    return os << "Kernel{t=" << kernel.now_ << ", " << kernel.scheduler_ << ", " << kernel.watchdog_ << ", "
              << kernel.trace_ << "}";
}

}
