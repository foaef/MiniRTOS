#pragma once

#include <iosfwd>
#include <memory>

#include "Config.hpp"
#include "Task.hpp"

// Decides the motor power: throttle * max power, faded out near the legal speed
// limit, reduced on a low battery, zero while braking. The power only rises by
// `rampW` per job so the bike does not jerk forward.
class AssistTask final : public Task {
    double maxPowerW_;
    double cutoffKmh_;
    double fadeKmh_;
    double rampW_;
    double lowBatteryFactor_;
    double lastCommandW_ = 0;

    void execute(Hal& hal, VehicleState& state) override;
    void onDeadlineMiss(Hal& hal, VehicleState& state) override;  // stale command: stop pushing
    void printSettings(std::ostream& os) const override;

public:
    AssistTask(const TaskSpec& spec, const Config& config);
    std::unique_ptr<Task> clone() const override;
};
