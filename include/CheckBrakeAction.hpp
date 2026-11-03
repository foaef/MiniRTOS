#pragma once

#include <string>

#include "TaskAction.hpp"

// Looks at the brake directly and cuts the motor at once.
class CheckBrakeAction final : public TaskAction {
public:
    void run(EBike& bike) const override;
    std::string name() const override;
};
