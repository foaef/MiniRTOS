#include "TaskSpec.hpp"

#include <ostream>
#include <utility>

#include "Errors.hpp"

TaskSpec::TaskSpec(std::string name, std::string type, int priority, int period, int wcet, int deadline)
    : name_(std::move(name)),
      type_(std::move(type)),
      priority_(priority),
      period_(period),
      wcet_(wcet),
      deadline_(deadline) {
    if (wcet_ <= 0 || wcet_ > deadline_ || deadline_ > period_) {
        throw ConfigError("task '" + name_ + "': need 0 < wcet <= deadline <= period");
    }
}

std::ostream& operator<<(std::ostream& os, const TaskSpec& spec) {
    os << spec.name_ << " (" << spec.type_ << ") priority=" << spec.priority_ << " period=" << spec.period_
       << " wcet=" << spec.wcet_ << " deadline=" << spec.deadline_;
    return os;
}
