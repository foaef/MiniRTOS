#include "RideStats.hpp"

#include <algorithm>
#include <iomanip>
#include <ostream>

void RideStats::record(double speedKmh, bool braking) {
    maxSpeedKmh_ = std::max(maxSpeedKmh_, speedKmh);
    speedSumKmh_ = speedSumKmh_ + speedKmh;
    ticks_++;
    if (braking) {
        brakingTicks_++;
    }
}

double RideStats::averageSpeedKmh() const {
    if (ticks_ == 0) {
        return 0;
    }
    return speedSumKmh_ / ticks_;
}

double RideStats::brakingShare() const {
    if (ticks_ == 0) {
        return 0;
    }
    return static_cast<double>(brakingTicks_) / ticks_;
}

std::ostream& operator<<(std::ostream& os, const RideStats& stats) {
    os << "RideStats{max " << std::fixed << std::setprecision(1) << stats.maxSpeedKmh() << " km/h, average "
       << stats.averageSpeedKmh() << " km/h, braking " << stats.brakingShare() * 100 << "% of " << stats.ticks_
       << " ticks}";
    return os;
}
