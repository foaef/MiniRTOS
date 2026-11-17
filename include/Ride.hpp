#pragma once

#include <iosfwd>
#include <string>
#include <vector>

// What the rider and the road do from `startMs` until the next step starts.
class RideStep {
    int startMs_;
    double riderPowerW_;   // how hard the rider pedals
    double throttle_;      // 0 = no assist wanted, 1 = full assist
    bool brake_;           // the rider holds the brake lever
    double slopePercent_;  // > 0 uphill, < 0 downhill

public:
    RideStep(int startMs, double riderPowerW, double throttle, bool brake, double slopePercent);

    int startMs() const { return startMs_; }
    double riderPowerW() const { return riderPowerW_; }
    double throttle() const { return throttle_; }
    bool brake() const { return brake_; }
    double slopePercent() const { return slopePercent_; }

    friend std::ostream& operator<<(std::ostream& os, const RideStep& step);
};

// A recorded ride, read from a csv file with the lines:
//   start_ms, rider_power_w, throttle, brake, slope_percent
// The first step starts at 0 ms and the start times keep increasing.
class Ride {
    std::vector<RideStep> steps_;

    void readStep(const std::string& line);

public:
    explicit Ride(const std::string& path);

    // The step active at time `ms` (the last step stays active forever).
    const RideStep& at(int ms) const;

    friend std::ostream& operator<<(std::ostream& os, const Ride& ride);
};
