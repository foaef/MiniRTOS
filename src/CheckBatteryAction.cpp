#include "CheckBatteryAction.hpp"

#include "EBike.hpp"

void CheckBatteryAction::run(EBike& bike) const { bike.checkBattery(); }

std::string CheckBatteryAction::name() const { return "check_battery"; }
