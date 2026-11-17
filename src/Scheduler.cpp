#include "Scheduler.hpp"

#include <iomanip>
#include <ostream>
#include <utility>

#include "Errors.hpp"

Scheduler::Scheduler(std::size_t traceCapacity) : trace_(traceCapacity) {}

Scheduler::Scheduler(const Scheduler& other)
    : trace_(other.trace_),
      now_(other.now_),
      idleTicks_(other.idleTicks_),
      switches_(other.switches_),
      lastRunning_(other.lastRunning_) {
    tasks_.reserve(other.tasks_.size());
    for (const auto& task : other.tasks_) {
        tasks_.push_back(task->clone());
    }
}

Scheduler& Scheduler::operator=(Scheduler other) {
    swap(*this, other);
    return *this;
}

void swap(Scheduler& first, Scheduler& second) noexcept {
    using std::swap;
    swap(first.tasks_, second.tasks_);
    swap(first.trace_, second.trace_);
    swap(first.now_, second.now_);
    swap(first.idleTicks_, second.idleTicks_);
    swap(first.switches_, second.switches_);
    swap(first.lastRunning_, second.lastRunning_);
}

void Scheduler::addTask(std::unique_ptr<Task> task) {
    for (const auto& existing : tasks_) {
        if (existing->spec().name() == task->spec().name()) {
            throw SchedulingError("two tasks are called '" + task->spec().name() + "'");
        }
        if (existing->spec().priority() == task->spec().priority()) {
            throw SchedulingError("'" + existing->spec().name() + "' and '" + task->spec().name() +
                                  "' have the same priority");
        }
    }
    tasks_.push_back(std::move(task));
}

int Scheduler::pickNext() const {
    int best = -1;
    for (std::size_t i = 0; i < tasks_.size(); i++) {
        const Task& task = *tasks_[i];
        if (task.isActive() &&
            (best < 0 || task.spec().priority() > tasks_[static_cast<std::size_t>(best)]->spec().priority())) {
            best = static_cast<int>(i);
        }
    }
    return best;
}

void Scheduler::tick(Hal& hal, VehicleState& state) {
    for (const auto& task : tasks_) {
        task->releaseIfDue(now_);
    }
    for (const auto& task : tasks_) {
        task->checkDeadline(now_, hal, state);
    }

    const int next = pickNext();
    if (next < 0) {
        idleTicks_++;
    } else {
        if (next != lastRunning_) {
            switches_++;
        }
        tasks_[static_cast<std::size_t>(next)]->runOneTick(now_, hal, state);
    }
    lastRunning_ = next;
    trace_.record(next);
    now_++;
}

std::vector<int> Scheduler::responseTimeBounds() const {
    std::vector<int> bounds;
    bounds.reserve(tasks_.size());
    for (const auto& task : tasks_) {
        const TaskSpec& spec = task->spec();
        int response = spec.wcet();
        while (response <= spec.deadline()) {
            int next = spec.wcet();
            for (const auto& other : tasks_) {
                const TaskSpec& higher = other->spec();
                if (higher.priority() > spec.priority()) {
                    const int releases = (response + higher.period() - 1) / higher.period();  // rounded up
                    next += releases * higher.wcet();
                }
            }
            if (next == response) {
                break;
            }
            response = next;
        }
        bounds.push_back(response);
    }
    return bounds;
}

bool Scheduler::isSchedulable() const {
    const std::vector<int> bounds = responseTimeBounds();
    for (std::size_t i = 0; i < tasks_.size(); i++) {
        if (bounds[i] > tasks_[i]->spec().deadline()) {
            return false;
        }
    }
    return true;
}

double Scheduler::cpuLoad() const {
    if (now_ == 0) {
        return 0;
    }
    return static_cast<double>(now_ - idleTicks_) / now_;
}

int Scheduler::totalMisses() const {
    int total = 0;
    for (const auto& task : tasks_) {
        total += task->misses();
    }
    return total;
}

std::ostream& operator<<(std::ostream& os, const Scheduler& scheduler) {
    os << "Scheduler{tick " << scheduler.now_ << ", CPU load " << std::fixed << std::setprecision(1)
       << scheduler.cpuLoad() * 100 << "%, " << scheduler.switches_ << " context switches, "
       << scheduler.totalMisses() << " missed deadlines}\n";
    const std::vector<int> bounds = scheduler.responseTimeBounds();
    for (std::size_t i = 0; i < scheduler.tasks_.size(); i++) {
        os << "  [" << i << "] " << *scheduler.tasks_[i] << "\n    response bound on paper=" << bounds[i] << "\n";
    }
    os << "  trace " << scheduler.trace_;
    return os;
}
