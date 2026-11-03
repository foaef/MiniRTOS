#pragma once

#include <string>

#include "TaskAction.hpp"

class ControlMotorAction final : public TaskAction {
public:
    void run(EBike& bike) const override;
    std::string name() const override;
};
