#pragma once

#include <iosfwd>
#include <string>
#include <vector>


// A broken sensor, injected by the ride file to test the firmware's fault detection.
enum class SensorFault { NONE, SPEED_LOST, THROTTLE_SHORTED };

std::ostream& operator<<(std::ostream& os, SensorFault fault);

// What the rider, the road and the sensors do from `startMs` until the next step starts.
class RideStep {
    int startMs_;
    double riderPowerW_;   // how hard the rider pedals
    double cadenceRpm_;    // pedal turns per minute (0 = not pedalling)
    double throttle_;      // 0..1, how far the throttle is twisted
    bool brake_;           // the rider holds the brake lever
    double slopePercent_;  // > 0 uphill, < 0 downhill
    SensorFault fault_;

public:
    RideStep(int startMs, double riderPowerW, double cadenceRpm, double throttle, bool brake, double slopePercent,
             SensorFault fault);

    // "ok", "speed_lost" or "throttle_shorted"
    static SensorFault faultFromText(const std::string& text);

    int startMs() const { return startMs_; }
    double riderPowerW() const { return riderPowerW_; }
    double cadenceRpm() const { return cadenceRpm_; }
    double throttle() const { return throttle_; }
    bool brake() const { return brake_; }
    double slopePercent() const { return slopePercent_; }
    SensorFault fault() const { return fault_; }

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
