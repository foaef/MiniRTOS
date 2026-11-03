#pragma once

#include <string>

#include "TaskAction.hpp"

class CheckBatteryAction final : public TaskAction {
public:
    void run(EBike& bike) const override;
    std::string name() const override;
};
