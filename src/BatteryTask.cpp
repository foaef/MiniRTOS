#include "BatteryTask.hpp"

#include <algorithm>
#include <ostream>
#include <string>

#include "Errors.hpp"
#include "Hal.hpp"
#include "VehicleState.hpp"

BatteryTask::BatteryTask(const TaskSpec& spec, const Config& config)
    : Task(spec),
      cells_(config.whole("battery.cells")),
      cellEmptyV_(config.number("battery.cell_empty_v")),
      cellFullV_(config.number("battery.cell_full_v")),
      internalResistanceOhm_(config.number("battery.internal_resistance_ohm")),
      lowPercent_(config.number("battery.low_percent")),
      cutoffCellV_(config.number("battery.cutoff_cell_v")) {
    if (cells_ == 0 || cellFullV_ <= cellEmptyV_ || internalResistanceOhm_ < 0 || lowPercent_ < 0 ||
        lowPercent_ > 100 || cutoffCellV_ < cellEmptyV_ || cutoffCellV_ >= cellFullV_) {
        throw ConfigError("battery values are out of range");
    }
}

std::unique_ptr<Task> BatteryTask::clone() const { return std::make_unique<BatteryTask>(*this); }

void BatteryTask::execute(Hal& hal, VehicleState& state) {
    const double packVoltage = hal.packVoltage();
    if (packVoltage / cells_ <= cutoffCellV_) {
        const int cellMillivolts = static_cast<int>(packVoltage / cells_ * 1000);
        state.reportFault("battery below the cutoff voltage (" + std::to_string(cellMillivolts) + " mV per cell)");
    }

    const double currentA = state.motorPowerW() / packVoltage;
    const double cellVoltage = (packVoltage + currentA * internalResistanceOhm_) / cells_;
    const double percent = std::clamp((cellVoltage - cellEmptyV_) / (cellFullV_ - cellEmptyV_) * 100, 0.0, 100.0);
    const bool low = percent <= lowPercent_;
    if (low && !state.lowBattery()) {
        lowWarnings_++;
    }
    state.updateBattery(percent, low);
}

void BatteryTask::printSettings(std::ostream& os) const {
    os << cells_ << " cells, " << internalResistanceOhm_ * 1000 << " mOhm, low below " << lowPercent_ << "%, cutoff at "
       << cutoffCellV_ << " V per cell, " << lowWarnings_ << " low warnings";
}