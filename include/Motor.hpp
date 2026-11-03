#pragma once

#include <iosfwd>

class Motor {
    double maxPowerW_;
    double cutoffKmh_;  // above this speed the motor must give no power (legal limit)
    double fadeKmh_;    // the power fades out over this many km/h before the cutoff
    double powerW_ = 0;

public:
    Motor(double maxPowerW, double cutoffKmh, double fadeKmh);

    void assist(double riderPowerW, double ratio, double speedKmh);

    void cut() { powerW_ = 0; }

    double powerW() const { return powerW_; }

    friend std::ostream& operator<<(std::ostream& os, const Motor& motor);
};
