#include "EBike.hpp"

#include <algorithm>
#include <iomanip>
#include <ostream>

#include "ConfigError.hpp"

EBike::EBike(const Config& cfg)
    : secondsPerTick_(tickLength(cfg)),
      battery_(cfg.whole("battery.cells"), cfg.number("battery.capacity_mah"), cfg.number("battery.cell_full_v"),
               cfg.number("battery.cell_empty_v"), cfg.number("battery.initial_charge")),
      motor_(cfg.number("motor.max_power_w"), cfg.number("motor.cutoff_kmh"), cfg.number("motor.fade_kmh")),
      massKg_(cfg.number("bike.mass_kg")),
      rollingResistance_(cfg.number("bike.rolling_resistance")),
      dragArea_(cfg.number("bike.drag_area_m2")),
      brakeDecel_(cfg.number("bike.brake_decel_ms2")),
      riderPowerW_(cfg.number("rider.power_w")),
      assistRatio_(cfg.number("assist.ratio")),
      lowBatteryLevel_(cfg.number("battery.low_level")),
      lowBatteryFactor_(cfg.number("battery.low_power_factor")),
      brakeEvery_(cfg.whole("rider.brake_every")),
      brakeDuration_(cfg.whole("rider.brake_duration")),
      speedMs_(cfg.number("bike.initial_speed_kmh") / 3.6) {
    if (massKg_ <= 0) {
        throw ConfigError("bike mass must be positive");
    }
}

bool EBike::brakePressedAt(int tick) const {
    if (brakeEvery_ == 0 || tick < brakeEvery_) {
        return false;
    }
    return tick % brakeEvery_ < brakeDuration_;
}

double EBike::tickLength(const Config& cfg) {
    const double hz = cfg.number("kernel.tick_hz");
    if (hz <= 0) {
        throw ConfigError("kernel.tick_hz must be positive");
    }
    return 1.0 / hz;
}

// read_sensors: copy the real speed and brake state into the "sensed" values.
void EBike::readSensors() {
    sensedSpeedKmh_ = speedMs_ * 3.6;
    sensedBrake_ = brakePressedAt(now_);
}

// control_motor: decide the motor power using only the sensed values.
void EBike::controlMotor() {
    if (sensedBrake_) {
        motor_.cut();
        return;
    }
    double ratio = assistRatio_;
    if (lowBattery_) {
        ratio = ratio * lowBatteryFactor_;
    }
    motor_.assist(riderPowerW_, ratio, sensedSpeedKmh_);
}

// check_brake: look at the brake directly and cut the motor at once.
void EBike::checkBrake() {
    const bool pressed = brakePressedAt(now_);
    if (pressed) {
        motor_.cut();
        if (!wasBraking_) {
            brakeCuts_++;
        }
    }
    wasBraking_ = pressed;
}

// check_battery: remember if the battery is low.
void EBike::checkBattery() { lowBattery_ = battery_.chargeLevel() <= lowBatteryLevel_; }

// New speed = old speed + acceleration * time.
void EBike::physicsStep(int now) {
    now_ = now;
    const double speed = std::max(speedMs_, 1.0);  // avoids dividing by zero when starting
    const double driveForce = (riderPowerW_ + motor_.powerW()) / speed;
    const double resistance = massKg_ * GRAVITY * rollingResistance_ + 0.5 * AIR_DENSITY * dragArea_ * speedMs_ * speedMs_;
    double acceleration = (driveForce - resistance) / massKg_;
    if (brakePressedAt(now)) {
        acceleration = acceleration - brakeDecel_;
    }
    speedMs_ = std::max(0.0, speedMs_ + acceleration * secondsPerTick_);
    distanceKm_ = distanceKm_ + speedMs_ * secondsPerTick_ / 1000;
    battery_.discharge(motor_.powerW(), secondsPerTick_);
}

std::ostream& operator<<(std::ostream& os, const EBike& bike) {
    os << "EBike{" << std::fixed << std::setprecision(1) << bike.speedMs_ * 3.6 << " km/h, "
       << std::setprecision(3) << bike.distanceKm_ << " km, " << bike.battery_ << ", " << bike.motor_
       << ", brake cut-offs=" << bike.brakeCuts_ << "}";
    return os;
}
