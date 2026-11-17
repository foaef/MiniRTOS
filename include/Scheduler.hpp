#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <vector>

#include "Task.hpp"
#include "TraceLog.hpp"

class Hal;
class VehicleState;

// Fixed-priority preemptive scheduler. Every tick it:
//   1. releases the jobs whose time has come
//   2. checks deadlines (a late task gets a chance to react, e.g. cut the motor)
//   3. gives 1 tick of CPU to the active task with the highest priority
// Preemption needs no extra code: a more important task released in the meantime
// simply wins the next pick, and the interrupted job continues later.
class Scheduler {
    std::vector<std::unique_ptr<Task>> tasks_;
    TraceLog trace_;
    int now_ = 0;
    int idleTicks_ = 0;
    int switches_ = 0;     // how often the running task changed
    int lastRunning_ = -1;

    int pickNext() const;  // index of the task to run, -1 if none is active

public:
    explicit Scheduler(std::size_t traceCapacity);
    Scheduler(const Scheduler& other);
    Scheduler& operator=(Scheduler other);
    ~Scheduler() = default;

    // Throws SchedulingError when the name or the priority is already used.
    void addTask(std::unique_ptr<Task> task);

    void tick(Hal& hal, VehicleState& state);

    // Worst response time of each task, worked out on paper (response time analysis):
    //   R = wcet + sum over more important tasks of ceil(R / their period) * their wcet,
    // repeated until R stops changing. Stops early once R passes the deadline.
    std::vector<int> responseTimeBounds() const;
    bool isSchedulable() const;  // every bound fits its deadline

    double cpuLoad() const;
    int totalMisses() const;

    friend void swap(Scheduler& first, Scheduler& second) noexcept;
    friend std::ostream& operator<<(std::ostream& os, const Scheduler& scheduler);
};
