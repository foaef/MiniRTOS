#pragma once

#include <iosfwd>
#include <string>

// One "task" line of the config file:
//   task <name> <type> <priority> <period> <wcet> <deadline>
// Times are in ticks (1 tick = 1 ms). Higher priority number = more important.
//   period   = a new job is released every `period` ticks
//   wcet     = worst-case execution time: CPU ticks one job needs
//   deadline = a job must be finished this many ticks after its release
class TaskSpec {
    std::string name_;
    std::string type_;
    int priority_;
    int period_;
    int wcet_;
    int deadline_;

public:
    TaskSpec(std::string name, std::string type, int priority, int period, int wcet, int deadline);

    const std::string& name() const { return name_; }
    const std::string& type() const { return type_; }
    int priority() const { return priority_; }
    int period() const { return period_; }
    int wcet() const { return wcet_; }
    int deadline() const { return deadline_; }

    friend std::ostream& operator<<(std::ostream& os, const TaskSpec& spec);
};
