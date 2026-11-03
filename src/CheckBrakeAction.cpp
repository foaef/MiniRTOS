#include "CheckBrakeAction.hpp"

#include "EBike.hpp"

void CheckBrakeAction::run(EBike& bike) const { bike.checkBrake(); }

std::string CheckBrakeAction::name() const { return "check_brake"; }
