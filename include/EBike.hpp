#pragma once

#include <iosfwd>

#include "Battery.hpp"
#include "Brake.hpp"
#include "Config.hpp"
#include "Motor.hpp"
#include "Physics.hpp"
#include "RideStats.hpp"
#include "Rider.hpp"

class EBike {
    double secondsPerTick_;
    Battery battery_;
    Motor motor_;
    Rider rider_;
    Physics physics_;
    Brake brake_;
    double assistRatio_;
    double lowBatteryLevel_;
    double lowBatteryFactor_;

    double speedMs_;
    double distanceKm_ = 0;
    int now_ = 0;
    bool lowBattery_ = false;
    RideStats stats_;

    double sensedSpeedKmh_ = 0;
    bool sensedBrake_ = false;

    static double tickLength(const Config& cfg);

public:
    explicit EBike(const Config& cfg);

    void readSensors();
    void controlMotor();
    void checkBrake();
    void checkBattery();

    void physicsStep(int now);

    double speedKmh() const { return speedMs_ * 3.6; }
    double chargeLevel() const { return battery_.chargeLevel(); }
    double motorPowerW() const { return motor_.powerW(); }

    friend std::ostream& operator<<(std::ostream& os, const EBike& bike);
};
