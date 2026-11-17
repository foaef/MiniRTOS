#include "SensorTask.hpp"

#include <algorithm>
#include <cmath>
#include <ostream>

#include "Errors.hpp"
#include "Hal.hpp"
#include "VehicleState.hpp"

namespace {
constexpr double THROTTLE_MARGIN_V = 0.3;  // tolerance for sensor and ADC noise
}

SensorTask::SensorTask(const TaskSpec& spec, const Config& config)
    : Task(spec),
      filter_(config.number("sensor.filter")),
      throttleMinV_(config.number("throttle.min_v")),
      throttleMaxV_(config.number("throttle.max_v")),
      maxSpeedStepKmh_(config.number("sensor.max_speed_step_kmh")) {
    if (filter_ <= 0 || filter_ > 1) {
        throw ConfigError("sensor.filter must be in (0, 1]");
    }
    if (throttleMinV_ < 0 || throttleMaxV_ <= throttleMinV_ || maxSpeedStepKmh_ <= 0) {
        throw ConfigError("throttle voltages or sensor.max_speed_step_kmh are out of range");
    }
}

std::unique_ptr<Task> SensorTask::clone() const { return std::make_unique<SensorTask>(*this); }

double SensorTask::throttleFromVoltage(double volts, VehicleState& state) const {
    if (volts < throttleMinV_ - THROTTLE_MARGIN_V || volts > throttleMaxV_ + THROTTLE_MARGIN_V) {
        state.reportFault("throttle signal out of range");
        return 0;
    }
    return std::clamp((volts - throttleMinV_) / (throttleMaxV_ - throttleMinV_), 0.0, 1.0);
}

void SensorTask::execute(Hal& hal, VehicleState& state) {
    // A real bike cannot lose more speed than this between two readings, even braking
    // hard, so a bigger drop means the wheel speed sensor stopped working.
    const double rawSpeed = hal.wheelSpeedKmh();
    if (std::abs(rawSpeed - lastRawSpeedKmh_) > maxSpeedStepKmh_) {
        state.reportFault("wheel speed jumped from " + std::to_string(static_cast<int>(lastRawSpeedKmh_)) + " to " +
                          std::to_string(static_cast<int>(rawSpeed)) + " km/h");
    }
    lastRawSpeedKmh_ = rawSpeed;

    const double throttle = throttleFromVoltage(hal.throttleVoltage(), state);
    state.updateMotion(state.speedKmh() + filter_ * (rawSpeed - state.speedKmh()),
                       state.throttle() + filter_ * (throttle - state.throttle()), hal.pedalCadenceRpm());
}

void SensorTask::printSettings(std::ostream& os) const {
    os << "filter " << filter_ << ", throttle " << throttleMinV_ << "-" << throttleMaxV_ << " V, max speed step "
       << maxSpeedStepKmh_ << " km/h";
}
