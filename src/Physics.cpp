#include "Physics.hpp"

#include <algorithm>
#include <iomanip>
#include <ostream>

#include "ConfigError.hpp"

Physics::Physics(double massKg, double rollingResistance, double dragArea, double gravity, double airDensity)
    : massKg_(massKg),
      rollingResistance_(rollingResistance),
      dragArea_(dragArea),
      gravity_(gravity),
      airDensity_(airDensity) {
    if (massKg_ <= 0) {
        throw ConfigError("bike mass must be positive");
    }
    if (rollingResistance_ < 0 || dragArea_ < 0 || gravity_ <= 0 || airDensity_ < 0) {
        throw ConfigError("invalid physics parameters");
    }
}

double Physics::driveForce(double powerW, double speedMs) const {
    const double speed = std::max(speedMs, 1.0);
    return powerW / speed;
}

double Physics::resistanceForce(double speedMs) const {
    return massKg_ * gravity_ * rollingResistance_ + 0.5 * airDensity_ * dragArea_ * speedMs * speedMs;
}

double Physics::acceleration(double powerW, double speedMs) const {
    return (driveForce(powerW, speedMs) - resistanceForce(speedMs)) / massKg_;
}

std::ostream& operator<<(std::ostream& os, const Physics& physics) {
    os << std::defaultfloat << std::setprecision(6) << "Physics{" << physics.massKg_ << " kg, rolling " << physics.rollingResistance_ << ", drag area "
       << physics.dragArea_ << " m2, g " << physics.gravity_ << ", air " << physics.airDensity_ << " kg/m3}";
    return os;
}
