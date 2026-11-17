#pragma once

#include <iosfwd>
#include <memory>
#include <string>

#include "TaskSpec.hpp"

class Hal;
class VehicleState;

// A periodic task. The kernel part (releases, CPU ticks, deadlines) is the same for
// every task and lives here; what a task actually does is in the derived classes.
// A job does its work in the tick it completes, so its output is at most `deadline`
// ticks late, which is exactly what the deadline promises.
class Task {
    TaskSpec spec_;
    int nextRelease_ = 0;
    int releasedAt_ = 0;
    int remaining_ = 0;  // CPU ticks the current job still needs
    bool active_ = false;
    bool missed_ = false;  // the current job already missed its deadline
    int jobs_ = 0;
    int misses_ = 0;
    int overruns_ = 0;  // releases skipped because the previous job was still running
    int worstResponse_ = 0;

    // What the task does when a job finishes.
    virtual void execute(Hal& hal, VehicleState& state) = 0;
    // What the task does when a job misses its deadline (default: nothing).
    virtual void onDeadlineMiss(Hal& hal, VehicleState& state);
    // Extra lines for operator<< (the task's own settings).
    virtual void printSettings(std::ostream& os) const = 0;

protected:
    explicit Task(TaskSpec spec);
    Task(const Task&) = default;
    Task& operator=(const Task&) = default;

public:
    virtual ~Task() = default;
    virtual std::unique_ptr<Task> clone() const = 0;

    const TaskSpec& spec() const { return spec_; }
    bool isActive() const { return active_; }
    int misses() const { return misses_; }

    // Kernel side, called by the scheduler once per tick.
    void releaseIfDue(int now);
    void checkDeadline(int now, Hal& hal, VehicleState& state);
    void runOneTick(int now, Hal& hal, VehicleState& state);

    friend std::ostream& operator<<(std::ostream& os, const Task& task);
};
