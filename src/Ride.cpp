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

std::ostream& operator<<(std::ostream& os, SensorFault fault) {
    switch (fault) {
        case SensorFault::NONE: return os << "ok";
        case SensorFault::SPEED_LOST: return os << "speed_lost";
        case SensorFault::THROTTLE_SHORTED: return os << "throttle_shorted";
    }
    return os << "unknown";
}

RideStep::RideStep(int startMs, double riderPowerW, double cadenceRpm, double throttle, bool brake,
                   double slopePercent, SensorFault fault)
    : startMs_(startMs),
      riderPowerW_(riderPowerW),
      cadenceRpm_(cadenceRpm),
      throttle_(throttle),
      brake_(brake),
      slopePercent_(slopePercent),
      fault_(fault) {
    if (startMs_ < 0 || riderPowerW_ < 0 || cadenceRpm_ < 0 || throttle_ < 0 || throttle_ > 1 ||
        std::abs(slopePercent_) > MAX_SLOPE_PERCENT) {
        throw ConfigError("ride step: need start, power, cadence >= 0, throttle in [0, 1], |slope| <= 30");
    }
}

SensorFault RideStep::faultFromText(const std::string& text) {
    if (text == "ok") {
        return SensorFault::NONE;
    }
    if (text == "speed_lost") {
        return SensorFault::SPEED_LOST;
    }
    if (text == "throttle_shorted") {
        return SensorFault::THROTTLE_SHORTED;
    }
    throw ConfigError("unknown sensor state '" + text + "' (use ok, speed_lost, throttle_shorted)");
}

std::ostream& operator<<(std::ostream& os, const RideStep& step) {
    os << "from " << step.startMs_ << " ms: rider " << step.riderPowerW_ << " W at " << step.cadenceRpm_
       << " rpm, throttle " << step.throttle_ << ", slope " << step.slopePercent_ << "%"
       << (step.brake_ ? ", braking" : "");
    if (step.fault_ != SensorFault::NONE) {
        os << ", sensor fault: " << step.fault_;
    }
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
    double cadence = 0;
    double throttle = 0;
    int brake = 0;
    double slope = 0;
    std::string sensors;
    if (!(words >> startMs >> power >> cadence >> throttle >> brake >> slope >> sensors)) {
        throw ConfigError("ride line needs: start_ms, rider_power_w, cadence_rpm, throttle, brake, slope_percent, "
                          "sensors");
    }
    if (!steps_.empty() && startMs <= steps_.back().startMs()) {
        throw ConfigError("ride start times must keep increasing");
    }
    steps_.emplace_back(startMs, power, cadence, throttle, brake != 0, slope, RideStep::faultFromText(sensors));
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
