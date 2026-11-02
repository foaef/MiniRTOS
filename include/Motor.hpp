#pragma once

#include <iosfwd>

// The motor adds power on top of what the rider pedals.
class Motor {
    double maxPowerW_;
    double cutoffKmh_;  // above this speed the motor must give no power (legal limit)
    double fadeKmh_;    // the power fades out over this many km/h before the cutoff
    double powerW_ = 0;

public:
    Motor(double maxPowerW, double cutoffKmh, double fadeKmh);

    // Motor power = a share of the rider's power, reduced when close to the speed limit.
    void assist(double riderPowerW, double ratio, double speedKmh);

    // Stop the motor immediately.
    void cut() { powerW_ = 0; }

    double powerW() const { return powerW_; }

    friend std::ostream& operator<<(std::ostream& os, const Motor& motor);
};
