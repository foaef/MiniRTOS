#include "ControlMotorAction.hpp"

#include "EBike.hpp"

void ControlMotorAction::run(EBike& bike) const { bike.controlMotor(); }

std::string ControlMotorAction::name() const { return "control_motor"; }
