#include "Brake.hpp"

#include <iomanip>
#include <ostream>

#include "ConfigError.hpp"

Brake::Brake(double decelerationMs2) : decelerationMs2_(decelerationMs2) {
    if (decelerationMs2_ <= 0) {
        throw ConfigError("brake deceleration must be positive");
    }
}

void Brake::update(bool pressed) {
    if (pressed && !wasPressed_) {
        presses_++;
    }
    wasPressed_ = pressed;
}

std::ostream& operator<<(std::ostream& os, const Brake& brake) {
    os << "Brake{" << std::fixed << std::setprecision(1) << brake.decelerationMs2_ << " m/s^2, " << brake.presses_
       << " presses}";
    return os;
}
