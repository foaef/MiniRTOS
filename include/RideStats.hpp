#pragma once

#include <iosfwd>

class RideStats {
    double maxSpeedKmh_ = 0;
    double speedSumKmh_ = 0;
    int ticks_ = 0;
    int brakingTicks_ = 0;

public:
    void record(double speedKmh, bool braking);

    double maxSpeedKmh() const { return maxSpeedKmh_; }

    double averageSpeedKmh() const;

    double brakingShare() const;

    friend std::ostream& operator<<(std::ostream& os, const RideStats& stats);
};
