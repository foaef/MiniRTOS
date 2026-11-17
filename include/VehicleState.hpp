#pragma once

#include <iosfwd>
#include <string>

// Data the tasks share: each value is written by one task and read by the others.
//   SensorTask  -> speed, throttle      BrakeTask   -> braking
//   BatteryTask -> charge, low battery  AssistTask  -> motor power
class VehicleState {
    double speedKmh_;
    double throttle_ = 0;
    double cadenceRpm_ = 0;
    std::string fault_;  // empty = no fault; a fault stays until the bike is restarted
    bool braking_ = false;
    double chargePercent_;
    bool lowBattery_ = false;
    double motorPowerW_ = 0;
    int brakePresses_ = 0;

public:
    VehicleState(double speedKmh, double chargePercent);

    void updateMotion(double measuredSpeedKmh, double measuredThrottle, double measuredCadenceRpm);
    void reportFault(const std::string& reason);  // only the first fault is kept
    void updateBrake(bool pressed);  // counts each new press
    void updateBattery(double chargePercent, bool low);
    void updateMotor(double powerW) { motorPowerW_ = powerW; }

    double speedKmh() const { return speedKmh_; }
    double throttle() const { return throttle_; }
    double cadenceRpm() const { return cadenceRpm_; }
    bool hasFault() const { return !fault_.empty(); }
    bool braking() const { return braking_; }
    bool lowBattery() const { return lowBattery_; }

    friend std::ostream& operator<<(std::ostream& os, const VehicleState& state);
};