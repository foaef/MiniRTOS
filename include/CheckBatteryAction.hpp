#pragma once

#include <string>

#include "TaskAction.hpp"

// Remembers if the battery is low.
class CheckBatteryAction final : public TaskAction {
public:
    void run(EBike& bike) const override;
    std::string name() const override;
};
