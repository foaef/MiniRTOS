#pragma once

#include <iosfwd>
#include <memory>

#include "Config.hpp"
#include "Task.hpp"

// Reads wheel speed, throttle and pedal cadence, checks that the readings are
// physically possible, and smooths them with a low-pass filter.
// A broken sensor is dangerous: a dead speed sensor reads 0 km/h, so the 25 km/h
// cut-off would never trigger; a shorted throttle reads "full power". Either one
// is reported as a fault, and AssistTask then keeps the motor off.
class SensorTask final : public Task {
    double filter_;          // 0..1: how much of each new reading is taken
    double throttleMinV_;    // throttle signal at rest
    double throttleMaxV_;    // throttle signal fully twisted
    double maxSpeedStepKmh_; // the largest change in speed possible between two readings
    double lastRawSpeedKmh_ = 0;

    void execute(Hal& hal, VehicleState& state) override;
    void printSettings(std::ostream& os) const override;

    // Throttle voltage -> 0..1. Reports a fault for a voltage outside the sensor's range.
    double throttleFromVoltage(double volts, VehicleState& state) const;

public:
    SensorTask(const TaskSpec& spec, const Config& config);
    std::unique_ptr<Task> clone() const override;
};
