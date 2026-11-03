#pragma once

#include <iosfwd>

class Physics {
    double massKg_;
    double rollingResistance_;
    double dragArea_;
    double gravity_;
    double airDensity_;

public:
    Physics(double massKg, double rollingResistance, double dragArea, double gravity, double airDensity);

    // The speed is never taken below 1 m/s, so the force stays finite at a standstill.
    double driveForce(double powerW, double speedMs) const;

    double resistanceForce(double speedMs) const;

    double acceleration(double powerW, double speedMs) const;

    friend std::ostream& operator<<(std::ostream& os, const Physics& physics);
};
