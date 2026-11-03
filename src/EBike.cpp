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
      rider_(cfg.number("rider.power_w"), cfg.whole("rider.brake_every"), cfg.whole("rider.brake_duration")),
      physics_(cfg.number("bike.mass_kg"), cfg.number("bike.rolling_resistance"), cfg.number("bike.drag_area_m2"),
               cfg.number("world.gravity"), cfg.number("world.air_density")),
      brake_(cfg.number("bike.brake_decel_ms2")),
      assistRatio_(cfg.number("assist.ratio")),
      lowBatteryLevel_(cfg.number("battery.low_level")),
      lowBatteryFactor_(cfg.number("battery.low_power_factor")),
      speedMs_(cfg.number("bike.initial_speed_kmh") / 3.6) {}

double EBike::tickLength(const Config& cfg) {
    const double hz = cfg.number("kernel.tick_hz");
    if (hz <= 0) {
        throw ConfigError("kernel.tick_hz must be positive");
    }
    return 1.0 / hz;
}

void EBike::readSensors() {
    sensedSpeedKmh_ = speedKmh();
    sensedBrake_ = rider_.isBraking(now_);
}

void EBike::controlMotor() {
    if (sensedBrake_) {
        motor_.cut();
        return;
    }
    double ratio = assistRatio_;
    if (lowBattery_) {
        ratio = ratio * lowBatteryFactor_;
    }
    motor_.assist(rider_.powerW(), ratio, sensedSpeedKmh_);
}

void EBike::checkBrake() {
    const bool pressed = rider_.isBraking(now_);
    if (pressed) {
        motor_.cut();
    }
    brake_.update(pressed);
}

void EBike::checkBattery() { lowBattery_ = battery_.chargeLevel() <= lowBatteryLevel_; }

void EBike::physicsStep(int now) {
    now_ = now;
    const bool braking = rider_.isBraking(now);
    double acceleration = physics_.acceleration(rider_.powerW() + motor_.powerW(), speedMs_);
    if (braking) {
        acceleration = acceleration - brake_.deceleration();
    }
    speedMs_ = std::max(0.0, speedMs_ + acceleration * secondsPerTick_);
    distanceKm_ = distanceKm_ + speedMs_ * secondsPerTick_ / 1000;
    battery_.discharge(motor_.powerW(), secondsPerTick_);
    stats_.record(speedKmh(), braking);
}

std::ostream& operator<<(std::ostream& os, const EBike& bike) {
    os << "EBike{" << std::fixed << std::setprecision(1) << bike.speedKmh() << " km/h, " << std::setprecision(3)
       << bike.distanceKm_ << " km, " << bike.battery_ << ", " << bike.motor_ << ", " << bike.brake_ << "}\n  "
       << bike.rider_ << "\n  " << bike.physics_ << "\n  " << bike.stats_;
    return os;
}
