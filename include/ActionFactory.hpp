#pragma once

#include <memory>
#include <string>

#include "TaskAction.hpp"

// Creates the action for a task type written in the config file
// ("read_sensors", "control_motor", "check_brake", "check_battery").
// Throws ConfigError for any other name.
std::shared_ptr<const TaskAction> makeAction(const std::string& type);
