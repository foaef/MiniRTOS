#include "BrakeTask.hpp"

#include <ostream>

#include "Hal.hpp"
#include "VehicleState.hpp"

BrakeTask::BrakeTask(const TaskSpec& spec) : Task(spec) {}

std::unique_ptr<Task> BrakeTask::clone() const { return std::make_unique<BrakeTask>(*this); }

void BrakeTask::execute(Hal& hal, VehicleState& state) {
    const bool pressed = hal.brakePressed();
    state.updateBrake(pressed);
    if (pressed) {
        hal.setMotorPower(0);
        state.updateMotor(0);
    }
}

void BrakeTask::onDeadlineMiss(Hal& hal, VehicleState& state) {
    hal.setMotorPower(0);
    state.updateMotor(0);
}

void BrakeTask::printSettings(std::ostream& os) const { os << "cuts the motor while braking"; }
