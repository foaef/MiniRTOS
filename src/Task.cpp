#include "Task.hpp"

#include <ostream>

#include "ActionFactory.hpp"
#include "ConfigError.hpp"
#include "SimulationError.hpp"

Task::Task(const std::string& name, const std::string& type, int priority, int period, int wcet, int deadline)
    : name_(name),
      action_(makeAction(type)),
      priority_(priority),
      period_(period),
      wcet_(wcet),
      deadline_(deadline) {
    if (period_ <= 0 || wcet_ <= 0 || wcet_ > deadline_ || deadline_ > period_) {
        throw ConfigError("task '" + name_ + "': need 0 < wcet <= deadline <= period");
    }
}

void Task::release(int now) {
    active_ = true;
    missed_ = false;
    remaining_ = wcet_;
    jobDeadline_ = now + deadline_;
    nextRelease_ = nextRelease_ + period_;
    jobs_++;
}

void Task::skipRelease() {
    nextRelease_ = nextRelease_ + period_;
    overruns_++;
}

bool Task::checkDeadline(int now) {
    if (active_ && !missed_ && now >= jobDeadline_) {
        missed_ = true;
        misses_++;
        return true;
    }
    return false;
}

void Task::runOneTick() {
    if (remaining_ <= 0) {
        throw SimulationError("task '" + name_ + "' has no work left to run");
    }
    remaining_--;
}

void Task::runAction(EBike& bike) const { action_->run(bike); }

std::ostream& operator<<(std::ostream& os, const Task& task) {
    os << task.name_ << " (" << *task.action_ << ") priority=" << task.priority_
       << " period=" << task.period_ << " wcet=" << task.wcet_ << " deadline=" << task.deadline_
       << " jobs=" << task.jobs_ << " missed=" << task.misses_ << " overruns=" << task.overruns_;
    return os;
}
