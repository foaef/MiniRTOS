#pragma once

#include <string>

#include "TaskAction.hpp"

// Decides the motor power from the sensed values.
class ControlMotorAction final : public TaskAction {
public:
    void run(EBike& bike) const override;
    std::string name() const override;
};
