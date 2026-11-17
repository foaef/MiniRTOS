#pragma once

#include <iosfwd>
#include <memory>

#include "Task.hpp"

// Cuts the motor the moment the brake lever is pulled. It runs often and with the
// highest priority because it is the safety function of the bike.
class BrakeTask final : public Task {
    void execute(Hal& hal, VehicleState& state) override;
    void onDeadlineMiss(Hal& hal, VehicleState& state) override;  // fail safe: motor off
    void printSettings(std::ostream& os) const override;

public:
    explicit BrakeTask(const TaskSpec& spec);
    std::unique_ptr<Task> clone() const override;
};
