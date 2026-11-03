#pragma once

#include <string>

#include "TaskAction.hpp"

// Copies the real speed and brake state into the values the controller sees.
class ReadSensorsAction final : public TaskAction {
public:
    void run(EBike& bike) const override;
    std::string name() const override;
};
