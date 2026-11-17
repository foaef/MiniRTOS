#include "SimHal.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <ostream>

#include "Errors.hpp"

namespace {
constexpr double SECONDS_PER_TICK = 0.001;
constexpr double SECONDS_PER_HOUR = 3600;
constexpr double KMH_PER_MS = 3.6;
constexpr double GRAVITY = 9.81;
constexpr double AIR_DENSITY = 1.2;
constexpr double MIN_SPEED_MS = 1.0;  // keeps power / speed finite when starting from rest
constexpr double SUPPLY_V = 5.0;       // a shorted throttle wire reads the full supply voltage
}  // namespace

SimHal::SimHal(const Config& config)
    : ride_(config.text("ride.file")),
      massKg_(config.number("bike.mass_kg")),
      rollingResistance_(config.number("bike.rolling_resistance")),
      dragAreaM2_(config.number("bike.drag_area_m2")),
      brakeDecelMs2_(config.number("bike.brake_decel_ms2")),
      ratedPowerW_(config.number("motor.rated_power_w")),
      cells_(config.whole("battery.cells")),
      cellEmptyV_(config.number("battery.cell_empty_v")),
      cellFullV_(config.number("battery.cell_full_v")),
      capacityAh_(config.number("battery.capacity_ah")),
      throttleMinV_(config.number("throttle.min_v")),
      throttleMaxV_(config.number("throttle.max_v")),
      chargeAh_(config.number("battery.capacity_ah") * config.number("battery.initial_charge")),
      speedMs_(config.number("bike.initial_speed_kmh") / KMH_PER_MS) {
    if (massKg_ <= 0 || brakeDecelMs2_ <= 0 || ratedPowerW_ <= 0 || cells_ == 0 || capacityAh_ <= 0 ||
        cellFullV_ <= cellEmptyV_ || throttleMinV_ < 0 || throttleMaxV_ <= throttleMinV_ || chargeAh_ < 0 || chargeAh_ > capacityAh_ || speedMs_ < 0) {
        throw ConfigError("bike or battery values are out of range");
    }
}

void SimHal::step(int nowMs) {
    const RideStep& now = ride_.at(nowMs);
    brake_ = now.brake();
    throttle_ = now.throttle();
    cadenceRpm_ = now.cadenceRpm();
    fault_ = now.fault();

    if (brake_ && motorPowerW_ > 0) {
        unsafeTicks_++;
        unsafeRun_++;
        worstUnsafeRun_ = std::max(worstUnsafeRun_, unsafeRun_);
    } else {
        unsafeRun_ = 0;
    }

    // Newton: pedals and motor push, rolling resistance, the slope and the air push back.
    const double slope = std::atan(now.slopePercent() / 100);
    const double push = (now.riderPowerW() + motorPowerW_) / std::max(speedMs_, MIN_SPEED_MS);
    const double resist = massKg_ * GRAVITY * (rollingResistance_ * std::cos(slope) + std::sin(slope)) +
                          0.5 * AIR_DENSITY * dragAreaM2_ * speedMs_ * speedMs_;
    double acceleration = (push - resist) / massKg_;
    if (brake_) {
        acceleration -= brakeDecelMs2_;
    }
    speedMs_ = std::max(0.0, speedMs_ + acceleration * SECONDS_PER_TICK);

    const double currentA = motorPowerW_ / packVoltage();
    chargeAh_ = std::max(0.0, chargeAh_ - currentA * SECONDS_PER_TICK / SECONDS_PER_HOUR);
    motorEnergyWh_ += motorPowerW_ * SECONDS_PER_TICK / SECONDS_PER_HOUR;
    distanceKm_ += speedMs_ * SECONDS_PER_TICK / 1000;
    maxSpeedKmh_ = std::max(maxSpeedKmh_, speedMs_ * KMH_PER_MS);
}

double SimHal::wheelSpeedKmh() const {
    if (fault_ == SensorFault::SPEED_LOST) {
        return 0;  // the wheel magnet fell off: no more pulses
    }
    return speedMs_ * KMH_PER_MS;
}

double SimHal::throttleVoltage() const {
    if (fault_ == SensorFault::THROTTLE_SHORTED) {
        return SUPPLY_V;
    }
    return throttleMinV_ + throttle_ * (throttleMaxV_ - throttleMinV_);
}

double SimHal::packVoltage() const {
    const double cellVoltage = cellEmptyV_ + (cellFullV_ - cellEmptyV_) * (chargeAh_ / capacityAh_);
    return cellVoltage * cells_;
}

void SimHal::setMotorPower(double watts) {
    if (watts < 0 || watts > ratedPowerW_) {
        throw HardwareError("motor power must be between 0 and " + std::to_string(static_cast<int>(ratedPowerW_)) + " W");
    }
    motorPowerW_ = watts;
}

void SimHal::printStatus(std::ostream& os) const {
    os << "SimHal{" << std::fixed << std::setprecision(1) << speedMs_ * KMH_PER_MS << " km/h (max " << maxSpeedKmh_
       << "), motor " << motorPowerW_ << " W, battery " << packVoltage() << " V, " << std::setprecision(3)
       << distanceKm_ << " km, motor energy " << std::setprecision(2) << motorEnergyWh_ << " Wh, "
       << unsafeTicks_ << " ticks braking with the motor on, worst brake reaction " << worstUnsafeRun_ << " ms}\n"
       << std::defaultfloat << std::setprecision(6) << ride_;
}
