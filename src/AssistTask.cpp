#include "AssistTask.hpp"

#include <algorithm>
#include <ostream>

#include "Errors.hpp"
#include "Hal.hpp"
#include "VehicleState.hpp"

AssistTask::AssistTask(const TaskSpec& spec, const Config& config)
    : Task(spec),
      maxPowerW_(config.number("assist.max_power_w")),
      cutoffKmh_(config.number("assist.cutoff_kmh")),
      fadeKmh_(config.number("assist.fade_kmh")),
      minCadenceRpm_(config.number("assist.min_cadence_rpm")),
      walkAssistKmh_(config.number("assist.walk_assist_kmh")),
      rampW_(config.number("assist.ramp_w")),
      lowBatteryFactor_(config.number("assist.low_battery_factor")) {
    if (maxPowerW_ <= 0 || cutoffKmh_ <= 0 || fadeKmh_ <= 0 || rampW_ <= 0 || lowBatteryFactor_ < 0 ||
        lowBatteryFactor_ > 1 || minCadenceRpm_ <= 0 || walkAssistKmh_ < 0 || walkAssistKmh_ > cutoffKmh_) {
        throw ConfigError("assist values are out of range");
    }
}

std::unique_ptr<Task> AssistTask::clone() const { return std::make_unique<AssistTask>(*this); }

double AssistTask::targetPower(const VehicleState& state) const {
    if (state.braking() || state.hasFault()) {
        return 0;
    }
    double target = 0;
    if (state.cadenceRpm() >= minCadenceRpm_) {
        const double fade = std::clamp((cutoffKmh_ - state.speedKmh()) / fadeKmh_, 0.0, 1.0);
        target = maxPowerW_ * fade;
    }
    if (state.speedKmh() < walkAssistKmh_) {
        target = std::max(target, state.throttle() * maxPowerW_);
    }
    if (state.lowBattery()) {
        target *= lowBatteryFactor_;
    }
    return target;
}

void AssistTask::execute(Hal& hal, VehicleState& state) {
    // power may drop at once, but only rises by rampW_ per job
    lastCommandW_ = std::min(targetPower(state), lastCommandW_ + rampW_);
    hal.setMotorPower(lastCommandW_);
    state.updateMotor(lastCommandW_);
}

void AssistTask::onDeadlineMiss(Hal& hal, VehicleState& state) {
    lastCommandW_ = 0;
    hal.setMotorPower(0);
    state.updateMotor(0);
}

void AssistTask::printSettings(std::ostream& os) const {
    os << "max " << maxPowerW_ << " W from " << minCadenceRpm_ << " rpm, fades out " << cutoffKmh_ - fadeKmh_ << "-"
       << cutoffKmh_ << " km/h, throttle up to " << walkAssistKmh_ << " km/h, ramp " << rampW_ << " W/job";
}
