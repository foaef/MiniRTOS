#include "VehicleState.hpp"

#include <iomanip>
#include <ostream>

VehicleState::VehicleState(double speedKmh, double chargePercent)
    : speedKmh_(speedKmh), chargePercent_(chargePercent) {}

void VehicleState::updateMotion(double measuredSpeedKmh, double measuredThrottle) {
    speedKmh_ = measuredSpeedKmh;
    throttle_ = measuredThrottle;
}

void VehicleState::updateBrake(bool pressed) {
    if (pressed && !braking_) {
        brakePresses_++;
    }
    braking_ = pressed;
}

void VehicleState::updateBattery(double chargePercent, bool low) {
    chargePercent_ = chargePercent;
    lowBattery_ = low;
}

std::ostream& operator<<(std::ostream& os, const VehicleState& state) {
    os << "VehicleState{" << std::fixed << std::setprecision(1) << state.speedKmh_ << " km/h, throttle "
       << state.throttle_ << ", " << (state.braking_ ? "braking" : "not braking") << ", battery "
       << state.chargePercent_ << "%" << (state.lowBattery_ ? " (LOW)" : "") << ", motor " << state.motorPowerW_
       << " W, " << state.brakePresses_ << " brake presses}";
    return os;
}

