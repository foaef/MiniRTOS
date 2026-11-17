#pragma once

#include <iosfwd>
#include <memory>

#include "Config.hpp"
#include "Task.hpp"

// Decides the motor power the way an EU pedelec must (EN 15194):
//   - pedal assist: only while the rider pedals, faded out before the 25 km/h limit
//   - throttle: only as walk/start assist, up to 6 km/h
//   - nothing while braking or after a sensor fault; half power on a low battery
// The power only rises by `rampW` per job, so the bike does not jerk forward.
class AssistTask final : public Task {
    double maxPowerW_;
    double cutoffKmh_;
    double fadeKmh_;
    double minCadenceRpm_;    // below this the rider is not really pedalling
    double walkAssistKmh_;    // the throttle works only below this speed
    double rampW_;
    double lowBatteryFactor_;
    double lastCommandW_ = 0;

    void execute(Hal& hal, VehicleState& state) override;
    void onDeadlineMiss(Hal& hal, VehicleState& state) override;  // stale command: stop pushing
    void printSettings(std::ostream& os) const override;

    double targetPower(const VehicleState& state) const;

public:
    AssistTask(const TaskSpec& spec, const Config& config);
    std::unique_ptr<Task> clone() const override;
};