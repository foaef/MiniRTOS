#pragma once

#include <iosfwd>

// Data the tasks share: each value is written by one task and read by the others.
//   SensorTask  -> speed, throttle      BrakeTask   -> braking
//   BatteryTask -> charge, low battery  AssistTask  -> motor power
class VehicleState {
    double speedKmh_;
    double throttle_ = 0;
    bool braking_ = false;
    double chargePercent_;
    bool lowBattery_ = false;
    double motorPowerW_ = 0;
    int brakePresses_ = 0;

public:
    VehicleState(double speedKmh, double chargePercent);

    void updateMotion(double measuredSpeedKmh, double measuredThrottle);
    void updateBrake(bool pressed);
    void updateBattery(double chargePercent, bool low);
    void updateMotor(double powerW) { motorPowerW_ = powerW; }

    double speedKmh() const { return speedKmh_; }
    double throttle() const { return throttle_; }
    bool braking() const { return braking_; }
    bool lowBattery() const { return lowBattery_; }

    friend std::ostream& operator<<(std::ostream& os, const VehicleState& state);
};
