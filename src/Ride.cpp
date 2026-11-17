#include "Ride.hpp"

#include <algorithm>
#include <cmath>
#include <ostream>
#include <sstream>

#include "Config.hpp"
#include "Errors.hpp"

namespace {
constexpr double MAX_SLOPE_PERCENT = 30;
}

RideStep::RideStep(int startMs, double riderPowerW, double throttle, bool brake, double slopePercent)
    : startMs_(startMs), riderPowerW_(riderPowerW), throttle_(throttle), brake_(brake), slopePercent_(slopePercent) {
    if (startMs_ < 0 || riderPowerW_ < 0 || throttle_ < 0 || throttle_ > 1 ||
        std::abs(slopePercent_) > MAX_SLOPE_PERCENT) {
        throw ConfigError("ride step: need start >= 0, power >= 0, throttle in [0, 1], |slope| <= 30");
    }
}

std::ostream& operator<<(std::ostream& os, const RideStep& step) {
    os << "from " << step.startMs_ << " ms: rider " << step.riderPowerW_ << " W, throttle " << step.throttle_
       << ", slope " << step.slopePercent_ << "%" << (step.brake_ ? ", braking" : "");
    return os;
}

Ride::Ride(const std::string& path) {
    Config::forEachLine(path, [this](const std::string& line) { readStep(line); });
    if (steps_.empty() || steps_.front().startMs() != 0) {
        throw ConfigError(path + ": the ride must start with a step at 0 ms");
    }
}

void Ride::readStep(const std::string& line) {
    std::string spaced = line;
    std::replace(spaced.begin(), spaced.end(), ',', ' ');
    std::istringstream words(spaced);
    int startMs = 0;
    double power = 0;
    double throttle = 0;
    int brake = 0;
    double slope = 0;
    if (!(words >> startMs >> power >> throttle >> brake >> slope)) {
        throw ConfigError("ride line needs: start_ms, rider_power_w, throttle, brake, slope_percent");
    }
    if (!steps_.empty() && startMs <= steps_.back().startMs()) {
        throw ConfigError("ride start times must keep increasing");
    }
    steps_.emplace_back(startMs, power, throttle, brake != 0, slope);
}

const RideStep& Ride::at(int ms) const {
    const RideStep* current = &steps_.front();
    for (const RideStep& step : steps_) {
        if (step.startMs() <= ms) {
            current = &step;
        }
    }
    return *current;
}

std::ostream& operator<<(std::ostream& os, const Ride& ride) {
    os << "Ride{" << ride.steps_.size() << " steps}";
    for (const RideStep& step : ride.steps_) {
        os << "\n  " << step;
    }
    return os;
}
