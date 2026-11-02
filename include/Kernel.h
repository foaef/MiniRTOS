#ifndef KERNEL_H
#define KERNEL_H

#include "Common.h"
#include "Config.h"
#include "Trace.h"
#include <vector>
#include <functional>
#include <memory>
#include <string>

namespace ebike {

enum class TaskState { Dormant, Ready, Running, Blocked };

class Task {
    TaskSpec spec_;
    std::function<void()> body_;
    TaskState state_{TaskState::Dormant};
    Priority effectivePriority_;
    Tick nextRelease_{0};
    Tick jobRelease_{0};
    Tick jobDeadline_{0};
    Tick remaining_{0};
    Tick lastRun_{0};
    Tick runTicks_{0};
    Tick maxResponse_{0};
    bool missedThisJob_{false};
    std::size_t jobs_{0};
    std::size_t completed_{0};
    std::size_t misses_{0};
    std::size_t overruns_{0};

public:
    Task(TaskSpec spec, std::function<void()> body);
    
    bool usesBus() const { return spec_.usesBus(); }
    const std::string& name() const { return spec_.name(); }
    Priority effectivePriority() const { return effectivePriority_; }
    Tick lastRun() const { return lastRun_; }
    Tick runTicks() const { return runTicks_; }
    Tick maxResponse() const { return maxResponse_; }
    std::size_t jobs() const { return jobs_; }
    std::size_t completed() const { return completed_; }
    std::size_t misses() const { return misses_; }
    std::size_t overruns() const { return overruns_; }

    bool due(Tick now) const { return now >= nextRelease_; }
    bool pending() const { return state_ != TaskState::Dormant; }
    bool runnable() const { return state_ == TaskState::Ready || state_ == TaskState::Running; }
    bool finished() const { return pending() && remaining_ == 0; }
    bool overdue(Tick now) const { return pending() && now >= jobDeadline_; }

    void startJob(Tick now);
    void skipRelease();
    bool checkDeadline(Tick now);
    void runOneTick(Tick now);
    void complete(Tick now);
    void preempt();
    void block();
    void unblock();
    void boostPriority(Priority priority);
    void restorePriority();

    friend std::ostream& operator<<(std::ostream& os, const Task& task);
};

class Mutex {
    std::string name_;
    Task* owner_{nullptr};
    std::vector<Task*> waiters_;
    std::size_t contentions_{0};
    std::size_t inheritances_{0};

public:
    explicit Mutex(std::string name);
    bool isOwnedBy(const Task& task) const { return owner_ == &task; }
    std::size_t contentions() const { return contentions_; }
    std::size_t inheritances() const { return inheritances_; }
    bool tryLock(Task& task);
    std::size_t unlock(const Task& task);

    friend std::ostream& operator<<(std::ostream& os, const Mutex& mutex);
};

class SoftwareTimer {
    TimerSpec spec_;
    std::function<void()> action_;
    Tick next_;
    std::size_t fires_{0};

public:
    SoftwareTimer(TimerSpec spec, std::function<void()> action);
    bool fireIfDue(Tick now);
    std::size_t fires() const { return fires_; }
    friend std::ostream& operator<<(std::ostream& os, const SoftwareTimer& timer);
};

class TimerService {
    std::vector<SoftwareTimer> timers_;

public:
    void add(TimerSpec spec, std::function<void()> action);
    void tick(Tick now);
    friend std::ostream& operator<<(std::ostream& os, const TimerService& service);
};

class Watchdog {
    Tick timeout_;
    Tick lastFeed_{0};
    std::size_t resets_{0};

public:
    explicit Watchdog(Tick timeout);
    void feed(Tick now) { lastFeed_ = now; }
    bool expired(Tick now) const { return now - lastFeed_ > timeout_; }
    void reset(Tick now);
    std::size_t resets() const { return resets_; }

    friend std::ostream& operator<<(std::ostream& os, const Watchdog& dog);
};

class Scheduler {
    std::vector<std::unique_ptr<Task>> tasks_;
    Mutex bus_;
    Task* running_{nullptr};
    Tick ticks_{0};
    Tick idleTicks_{0};
    std::size_t switches_{0};

    void releaseJobs(Tick now, TraceBuffer& trace);
    void checkDeadlines(Tick now, TraceBuffer& trace);
    static bool moreUrgent(const Task& candidate, const Task& best);
    Task* pickNext(Tick now, TraceBuffer& trace);
    void finishJob(Task& task, Tick now, TraceBuffer& trace);

public:
    Scheduler();
    void addTask(TaskSpec spec, std::function<void()> body);
    void tick(Tick now, TraceBuffer& trace);
    bool hasOverdueJob(Tick now) const;
    std::size_t totalDeadlineMisses() const;
    double cpuUtilization() const;
    const std::vector<std::unique_ptr<Task>>& tasks() const { return tasks_; }
    const Mutex& bus() const { return bus_; }
    std::size_t contextSwitches() const { return switches_; }
    Tick idleTicks() const { return idleTicks_; }

    friend std::ostream& operator<<(std::ostream& os, const Scheduler& scheduler);
};

class Kernel {
    Scheduler scheduler_;
    TimerService timers_;
    Watchdog watchdog_;
    TraceBuffer trace_;
    Tick now_{0};

public:
    Kernel(std::size_t traceCapacity, Tick watchdogTimeout);
    void addTask(TaskSpec spec, std::function<void()> body);
    void addTimer(TimerSpec spec, std::function<void()> action);
    void feedWatchdogIfHealthy();
    void note(TraceKind kind, const std::string& text);
    void run(Tick duration, const std::function<void(Tick)>& hardwareTick, const std::function<void()>& onWatchdogReset);
    Tick now() const { return now_; }
    const Scheduler& scheduler() const { return scheduler_; }
    const TimerService& timers() const { return timers_; }
    const Watchdog& watchdog() const { return watchdog_; }
    const TraceBuffer& trace() const { return trace_; }

    friend std::ostream& operator<<(std::ostream& os, const Kernel& kernel);
};

}

#endif
