#pragma once

#include <iosfwd>

// Every `brakeEvery` ticks the brake is pressed for `brakeDuration` ticks,
// starting at tick `brakeEvery`. brakeEvery = 0 means the rider never brakes.
class Rider {
    double powerW_;
    int brakeEvery_;
    int brakeDuration_;

public:
    Rider(double powerW, int brakeEvery, int brakeDuration);

    double powerW() const { return powerW_; }

    bool isBraking(int tick) const;

    friend std::ostream& operator<<(std::ostream& os, const Rider& rider);
};
