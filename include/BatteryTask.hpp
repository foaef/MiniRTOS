#pragma once

#include <iosfwd>
#include <memory>

#include "Config.hpp"
#include "Task.hpp"

// Turns the pack voltage into a charge level and flags a low battery.
class BatteryTask final : public Task {
    int cells_;
    double cellEmptyV_;
    double cellFullV_;
    double lowPercent_;
    int lowWarnings_ = 0;

    void execute(Hal& hal, VehicleState& state) override;
    void printSettings(std::ostream& os) const override;

public:
    BatteryTask(const TaskSpec& spec, const Config& config);
    std::unique_ptr<Task> clone() const override;
};
