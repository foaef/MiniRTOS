#pragma once

#include <memory>
#include <string>

#include "TaskAction.hpp"

// Throws ConfigError for any other name.
std::shared_ptr<const TaskAction> makeAction(const std::string& type);
