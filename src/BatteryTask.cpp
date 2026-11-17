#include "BatteryTask.hpp"

#include <algorithm>
#include <ostream>

#include "Errors.hpp"
#include "Hal.hpp"
#include "VehicleState.hpp"

BatteryTask::BatteryTask(const TaskSpec& spec, const Config& config)
    : Task(spec),
      cells_(config.whole("battery.cells")),
      cellEmptyV_(config.number("battery.cell_empty_v")),
      cellFullV_(config.number("battery.cell_full_v")),
      lowPercent_(config.number("battery.low_percent")) {
    if (cells_ == 0 || cellFullV_ <= cellEmptyV_ || lowPercent_ < 0 || lowPercent_ > 100) {
        throw ConfigError("battery values are out of range");
    }
}

std::unique_ptr<Task> BatteryTask::clone() const { return std::make_unique<BatteryTask>(*this); }

void BatteryTask::execute(Hal& hal, VehicleState& state) {
    const double cellVoltage = hal.packVoltage() / cells_;
    const double percent = std::clamp((cellVoltage - cellEmptyV_) / (cellFullV_ - cellEmptyV_) * 100, 0.0, 100.0);
    const bool low = percent <= lowPercent_;
    if (low && !state.lowBattery()) {
        lowWarnings_++;
    }
    state.updateBattery(percent, low);
}

void BatteryTask::printSettings(std::ostream& os) const {
    os << cells_ << " cells, low below " << lowPercent_ << "%, " << lowWarnings_ << " low warnings";
}
