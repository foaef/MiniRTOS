#pragma once

#include <iosfwd>
#include <memory>
#include <string>

#include "TaskAction.hpp"

class EBike;

// One periodic task.
//   period   = a new job is released every `period` ticks
//   wcet     = how many ticks of CPU one job needs
//   deadline = a job must be finished this many ticks after its release
// The action (what to do when a job is done) never changes, so copies of a
// task simply share it; no special copy code is needed.
class Task {
    // fixed data, comes from the config file
    std::string name_;
    std::shared_ptr<const TaskAction> action_;
    int priority_;
    int period_;
    int wcet_;
    int deadline_;

    // changes while the simulation runs
    int nextRelease_ = 0;   // tick when the next job will be released
    int jobDeadline_ = 0;   // tick by which the current job must finish
    int remaining_ = 0;     // CPU ticks the current job still needs
    bool active_ = false;   // true while a job is waiting or running
    bool missed_ = false;   // true if the current job already missed its deadline
    int jobs_ = 0;
    int misses_ = 0;
    int overruns_ = 0;

public:
    // `type` is the task type from the config file; it selects the action.
    Task(const std::string& name, const std::string& type, int priority, int period, int wcet, int deadline);

    const std::string& name() const { return name_; }
    int priority() const { return priority_; }
    bool isActive() const { return active_; }
    int misses() const { return misses_; }

    // True when it is time to release a new job.
    bool isDue(int now) const { return now >= nextRelease_; }

    // True when the current job has received all the CPU time it needs.
    bool isFinished() const { return active_ && remaining_ == 0; }

    // Start a new job.
    void release(int now);

    // The next release arrived but the previous job is still not finished.
    void skipRelease();

    // Returns true (only once per job) if the job is still unfinished after its deadline.
    bool checkDeadline(int now);

    // Give the job one tick of CPU. Throws SimulationError if the job has no work left.
    void runOneTick();

    // Do the real work of the finished job (the derived action decides what that is).
    void runAction(EBike& bike) const;

    // The job is done, the task sleeps until its next release.
    void finish() { active_ = false; }

    friend std::ostream& operator<<(std::ostream& os, const Task& task);
};
