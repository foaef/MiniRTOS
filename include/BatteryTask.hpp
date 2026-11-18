#pragma once

#include <iosfwd>
#include <memory>

#include "Config.hpp"
#include "Task.hpp"

// Watches the battery pack:
//   - charge level: the measured voltage drops while the motor draws current, so the
//     drop on the internal resistance is added back before converting voltage to charge
//     (otherwise the charge would look lower every time the rider accelerates)
//   - low battery warning below `lowPercent` (AssistTask then halves the power)
//   - low-voltage cutoff: below `cutoffCellV` per cell the cells can be damaged, so a
//     fault is raised and the motor stays off
class BatteryTask final : public Task {
    int cells_;
    double cellEmptyV_;
    double cellFullV_;
    double internalResistanceOhm_;
    double lowPercent_;
    double cutoffCellV_;
    int lowWarnings_ = 0;

    void execute(Hal& hal, VehicleState& state) override;
    void printSettings(std::ostream& os) const override;

public:
    BatteryTask(const TaskSpec& spec, const Config& config);
    std::unique_ptr<Task> clone() const override;
};