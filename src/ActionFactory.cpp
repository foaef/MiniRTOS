#include "ActionFactory.hpp"

#include "CheckBatteryAction.hpp"
#include "CheckBrakeAction.hpp"
#include "ConfigError.hpp"
#include "ControlMotorAction.hpp"
#include "ReadSensorsAction.hpp"

std::shared_ptr<const TaskAction> makeAction(const std::string& type) {
    if (type == "read_sensors") {
        return std::make_shared<ReadSensorsAction>();
    }
    if (type == "control_motor") {
        return std::make_shared<ControlMotorAction>();
    }
    if (type == "check_brake") {
        return std::make_shared<CheckBrakeAction>();
    }
    if (type == "check_battery") {
        return std::make_shared<CheckBatteryAction>();
    }
    throw ConfigError("unknown task type '" + type + "'");
}
