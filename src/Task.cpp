#include "Task.hpp"

#include <algorithm>
#include <ostream>
#include <utility>

Task::Task(TaskSpec spec) : spec_(std::move(spec)) {}

void Task::onDeadlineMiss(Hal& /*hal*/, VehicleState& /*state*/) {}

void Task::releaseIfDue(int now) {
    if (now < nextRelease_) {
        return;
    }
    nextRelease_ += spec_.period();
    if (active_) {
        overruns_++;  // the previous job is still not done: skip this release
        return;
    }
    active_ = true;
    missed_ = false;
    remaining_ = spec_.wcet();
    releasedAt_ = now;
    jobs_++;
}

void Task::checkDeadline(int now, Hal& hal, VehicleState& state) {
    if (active_ && !missed_ && now >= releasedAt_ + spec_.deadline()) {
        missed_ = true;
        misses_++;
        onDeadlineMiss(hal, state);
    }
}

void Task::runOneTick(int now, Hal& hal, VehicleState& state) {
    remaining_--;
    if (remaining_ > 0) {
        return;
    }
    active_ = false;
    worstResponse_ = std::max(worstResponse_, now + 1 - releasedAt_);
    execute(hal, state);
}

std::ostream& operator<<(std::ostream& os, const Task& task) {
    os << task.spec_ << "\n    jobs=" << task.jobs_ << " missed=" << task.misses_ << " overruns=" << task.overruns_
       << " worst response=" << task.worstResponse_ << " | ";
    task.printSettings(os);
    return os;
}
