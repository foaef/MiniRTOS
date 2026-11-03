#pragma once

#include <iosfwd>
#include <string>

class EBike;

class TaskAction {
public:
    virtual ~TaskAction() = default;

    virtual void run(EBike& bike) const = 0;

    virtual std::string name() const = 0;

    friend std::ostream& operator<<(std::ostream& os, const TaskAction& action);
};
