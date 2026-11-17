#pragma once

#include <memory>

#include "Config.hpp"
#include "Task.hpp"

// Builds the right task for the `type` written in the config file
// (brake, sensors, assist or battery). Throws ConfigError for an unknown type.
std::unique_ptr<Task> makeTask(const TaskSpec& spec, const Config& config);
