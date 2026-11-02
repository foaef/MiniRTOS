#include "EBike.h"
#include "Config.h"
#include <cmath>
#include <algorithm>

namespace ebike {

EBike::EBike(const BikeConfig& cfg)
    : dtSeconds_(1.0 / cfg.number("kernel.tick_hz")),
      battery_(static_cast<int>(cfg.ticks("battery.cells_series")), cfg.number("battery.capacity_mah"),
               cfg.number("battery.cell_full_v"), cfg.number("battery.cell_empty_v"),
               cfg.number("battery.internal_resistance_ohm"), cfg.number("battery.initial_soc")),
      motor_(PidController(cfg.number("motor.pid_kp"), cfg.number("motor.pid_ki"), cfg.number("motor.pid_kd"), 0.0, 1.0),
             cfg.number("motor.max_power_w"), cfg.number("motor.cutoff_kmh"), cfg.number("motor.fade_kmh"),
             cfg.number("motor.derate_start_c"), cfg.number("motor.limit_c"), cfg.number("motor.heat_rate_c_per_s"),
             cfg.number("motor.cool_rate_per_s"), cfg.number("motor.response_per_s"), cfg.number("env.ambient_c")),
      dynamics_(cfg.number("bike.mass_kg"), cfg.number("bike.rolling_resistance"), cfg.number("bike.drag_area_m2"),
                cfg.number("bike.brake_decel_ms2"), cfg.number("bike.initial_speed_kmh")),
      rider_(cfg.number("rider.base_power_w"), cfg.number("rider.power_swing_w"), cfg.ticks("rider.power_period"),
             cfg.number("route.gradient_pct"), cfg.ticks("route.gradient_period"),
             cfg.ticks("rider.brake_every"), cfg.ticks("rider.brake_duration")),
      sensors_(cfg.ticks("sim.seed"), cfg.number("sensor.speed_noise_kmh"), cfg.number("sensor.power_noise_w"),
               cfg.number("sensor.voltage_noise_v")),
      assist_(cfg),
      level_(AssistTable::fromIndex(cfg.ticks("assist.initial"))),
      lowSoc_(cfg.number("battery.low_soc")),
      criticalSoc_(cfg.number("battery.critical_soc")),
      lowBatteryFactor_(cfg.number("battery.low_power_factor")),
      defaultWhPerKm_(cfg.number("battery.default_wh_per_km")),
      alertDisplayTicks_(cfg.ticks("display.alert_ticks")) {
    if (dtSeconds_ <= 0.0 || criticalSoc_ >= lowSoc_) {
        throw ConfigError("tick_hz must be positive and battery.critical_soc below battery.low_soc");
    }
}

double EBike::batteryFactor() const {
    switch (batteryState_) {
        case BatteryState::Normal: return 1.0;
        case BatteryState::Low: return lowBatteryFactor_;
        case BatteryState::Critical: return 0.0;
    }
    return 0.0;
}

const char* EBike::stateName(BatteryState state) {
    switch (state) {
        case BatteryState::Normal: return "battery ok";
        case BatteryState::Low: return "LOW BATTERY";
        case BatteryState::Critical: return "BATTERY CRITICAL";
    }
    return "?";
}

BatteryState EBike::classify(double soc) const {
    if (soc <= criticalSoc_) {
        return BatteryState::Critical;
    }
    return soc <= lowSoc_ ? BatteryState::Low : BatteryState::Normal;
}

double EBike::rangeKm() const {
    const double measured = stats_.whPerKm(kMinimumKmForConsumption);
    return battery_.estimatedRangeKm(measured > 0.0 ? measured : defaultWhPerKm_);
}

void EBike::physicsStep(Tick now) {
    clock_ = now;
    dynamics_.setRiderInput(rider_.pedalPowerAt(now), rider_.gradientAt(now), rider_.brakingAt(now));
    motor_.advance(dtSeconds_);
    dynamics_.step(dtSeconds_, motor_.powerW());
    const double current = battery_.currentForPower(motor_.powerW());
    stats_.accumulate(dynamics_.speedKmh(), current * battery_.openCircuitVoltage(), dtSeconds_);
    battery_.discharge(current, dtSeconds_);
}

void EBike::pollSensors() { samples_.push(sensors_.read(clock_, dynamics_, battery_, motor_, rider_)); }

void EBike::checkBrake() {
    const bool pressed = rider_.brakingAt(clock_);
    if (pressed) {
        motor_.cutPower();
        if (!braking_) {
            ++brakeCuts_;
        }
    }
    braking_ = pressed;
}

void EBike::runMotorControl() {
    while (std::optional<SensorSample> sample = samples_.pop()) {
        latest_ = sample;
    }
    const Tick elapsed = std::max<Tick>(clock_ - lastControl_, 1);
    lastControl_ = clock_;
    if (!latest_) {
        return;
    }
    if (safeMode_ || latest_->brake()) {
        motor_.cutPower();
        return;
    }
    const double target = motor_.targetPower(*latest_, assist_.ratio(level_), batteryFactor());
    motor_.regulate(target, static_cast<double>(elapsed) * dtSeconds_);
}

std::optional<std::string> EBike::monitorBattery() {
    const BatteryState next = classify(battery_.stateOfCharge());
    if (next == batteryState_) {
        return std::nullopt;
    }
    batteryState_ = next;
    const std::string text = stateName(next);
    alertHistory_.push_back("t=" + std::to_string(clock_) + " " + text);
    alerts_.push(text);
    return text;
}

void EBike::refreshDisplay() {
    while (std::optional<std::string> alert = alerts_.pop()) {
        display_.showAlert(*alert, clock_ + alertDisplayTicks_);
    }
    if (latest_) {
        display_.render(*latest_, battery_.stateOfCharge(), level_, rangeKm(), clock_);
    }
}

void EBike::logTelemetry() {
    telemetry_.push_back("t=" + std::to_string(clock_) + " " + Format::fixed(dynamics_.speedKmh(), 1) + " km/h, pedal " +
                         Format::fixed(dynamics_.pedalPowerW(), 0) + " W, motor " + Format::fixed(motor_.powerW(), 0) +
                         " W, " + Format::fixed(motor_.temperatureC(), 1) + " C, SoC " +
                         Format::percent(battery_.stateOfCharge()) + ", " + AssistTable::name(level_));
}

void EBike::cycleAssist() {
    if (!safeMode_) {
        level_ = AssistTable::next(level_);
    }
}

void EBike::enterSafeState() {
    safeMode_ = true;
    level_ = AssistLevel::Off;
    motor_.cutPower();
}

std::ostream& operator<<(std::ostream& os, const EBike& bike) {
    return os << "EBike{" << bike.battery_ << ", " << bike.motor_ << ", " << bike.dynamics_ << ", assist "
              << AssistTable::name(bike.level_) << ", " << bike.stats_ << ", " << bike.samples_ << "}";
}

}
