#pragma once

#include <iosfwd>
#include <memory>
#include <string>

#include "TaskAction.hpp"

class EBike;

//   period   = a new job is released every `period` ticks
//   wcet     = how many ticks of CPU one job needs
//   deadline = a job must be finished this many ticks after its release
// Copies of a task share its (unchanging) action, so no special copy code is needed.
class Task {
    std::string name_;
    std::shared_ptr<const TaskAction> action_;
    int priority_;
    int period_;
    int wcet_;
    int deadline_;

    int nextRelease_ = 0;
    int releasedAt_ = 0;
    int worstResponse_ = 0;
    int jobDeadline_ = 0;
    int remaining_ = 0;
    bool active_ = false;
    bool missed_ = false;
    int jobs_ = 0;
    int misses_ = 0;
    int overruns_ = 0;

public:
    Task(const std::string& name, const std::string& type, int priority, int period, int wcet, int deadline);

    const std::string& name() const { return name_; }
    int priority() const { return priority_; }
    int period() const { return period_; }
    int wcet() const { return wcet_; }
    int deadline() const { return deadline_; }

    // Longest time from release to completion of any finished job (0 = none finished yet).
    int worstResponse() const { return worstResponse_; }
    bool isActive() const { return active_; }
    int misses() const { return misses_; }
    bool isDue(int now) const { return now >= nextRelease_; }
    bool isFinished() const { return active_ && remaining_ == 0; }
    void release(int now);
    void skipRelease();
    bool checkDeadline(int now);
    void runOneTick();
    void runAction(EBike& bike) const;
    // The job is done. `now` is the tick in which its last CPU tick ran.
    void finish(int now);

    friend std::ostream& operator<<(std::ostream& os, const Task& task);
};
