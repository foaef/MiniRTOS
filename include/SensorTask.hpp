#pragma once

#include <iosfwd>
#include <memory>

#include "Config.hpp"
#include "Task.hpp"

// Reads wheel speed and throttle and smooths them with a low-pass filter
// (the raw readings on the real bike are noisy).
class SensorTask final : public Task {
    double filter_;  // 0..1: how much of each new reading is taken

    void execute(Hal& hal, VehicleState& state) override;
    void printSettings(std::ostream& os) const override;

public:
    SensorTask(const TaskSpec& spec, const Config& config);
    std::unique_ptr<Task> clone() const override;
};
