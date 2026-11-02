#include "Motor.hpp"

#include <algorithm>
#include <iomanip>
#include <ostream>

#include "ConfigError.hpp"

Motor::Motor(double maxPowerW, double cutoffKmh, double fadeKmh)
    : maxPowerW_(maxPowerW), cutoffKmh_(cutoffKmh), fadeKmh_(fadeKmh) {
    if (maxPowerW <= 0 || fadeKmh <= 0) {
        throw ConfigError("invalid motor parameters");
    }
}

void Motor::assist(double riderPowerW, double ratio, double speedKmh) {
    double fade = (cutoffKmh_ - speedKmh) / fadeKmh_;  // 1 = full power, 0 = none
    fade = std::max(0.0, std::min(1.0, fade));
    powerW_ = ratio * riderPowerW * fade;
    powerW_ = std::min(powerW_, maxPowerW_);
}

std::ostream& operator<<(std::ostream& os, const Motor& motor) {
    os << "Motor{" << std::fixed << std::setprecision(0) << motor.powerW_ << "/" << motor.maxPowerW_ << " W}";
    return os;
}
