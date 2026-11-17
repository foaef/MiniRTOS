#include "SensorTask.hpp"

#include <ostream>

#include "Errors.hpp"
#include "Hal.hpp"
#include "VehicleState.hpp"

SensorTask::SensorTask(const TaskSpec& spec, const Config& config)
    : Task(spec), filter_(config.number("sensor.filter")) {
    if (filter_ <= 0 || filter_ > 1) {
        throw ConfigError("sensor.filter must be in (0, 1]");
    }
}

std::unique_ptr<Task> SensorTask::clone() const { return std::make_unique<SensorTask>(*this); }

void SensorTask::execute(Hal& hal, VehicleState& state) {
    const double speed = state.speedKmh() + filter_ * (hal.wheelSpeedKmh() - state.speedKmh());
    const double throttle = state.throttle() + filter_ * (hal.throttle() - state.throttle());
    state.updateMotion(speed, throttle);
}

void SensorTask::printSettings(std::ostream& os) const { os << "filter " << filter_; }
