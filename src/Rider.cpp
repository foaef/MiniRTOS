#include "Rider.hpp"

#include <iomanip>
#include <ostream>

#include "ConfigError.hpp"

Rider::Rider(double powerW, int brakeEvery, int brakeDuration)
    : powerW_(powerW), brakeEvery_(brakeEvery), brakeDuration_(brakeDuration) {
    if (powerW_ < 0 || brakeEvery_ < 0 || brakeDuration_ < 0) {
        throw ConfigError("rider values cannot be negative");
    }
}

bool Rider::isBraking(int tick) const {
    if (brakeEvery_ == 0 || tick < brakeEvery_) {
        return false;
    }
    return tick % brakeEvery_ < brakeDuration_;
}

std::ostream& operator<<(std::ostream& os, const Rider& rider) {
    os << "Rider{" << std::fixed << std::setprecision(0) << rider.powerW_ << " W, ";
    if (rider.brakeEvery_ == 0) {
        os << "never brakes}";
    } else {
        os << "brakes every " << rider.brakeEvery_ << " ticks for " << rider.brakeDuration_ << " ticks}";
    }
    return os;
}
