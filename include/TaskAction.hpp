#pragma once

#include <iosfwd>
#include <string>

class EBike;

// What a task does when one of its jobs has received all its CPU time.
// Each kind of task is a derived class, so the scheduler never has to look at
// a type name: it just calls run() and the right version is executed.
class TaskAction {
public:
    virtual ~TaskAction() = default;

    // The real work of the task.
    virtual void run(EBike& bike) const = 0;

    // The name used in the config file, for example "read_sensors".
    virtual std::string name() const = 0;

    friend std::ostream& operator<<(std::ostream& os, const TaskAction& action);
};
