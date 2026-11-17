#include "TaskFactory.hpp"

#include "AssistTask.hpp"
#include "BatteryTask.hpp"
#include "BrakeTask.hpp"
#include "Errors.hpp"
#include "SensorTask.hpp"

std::unique_ptr<Task> makeTask(const TaskSpec& spec, const Config& config) {
    if (spec.type() == "brake") {
        return std::make_unique<BrakeTask>(spec);
    }
    if (spec.type() == "sensors") {
        return std::make_unique<SensorTask>(spec, config);
    }
    if (spec.type() == "assist") {
        return std::make_unique<AssistTask>(spec, config);
    }
    if (spec.type() == "battery") {
        return std::make_unique<BatteryTask>(spec, config);
    }
    throw ConfigError("task '" + spec.name() + "' has unknown type '" + spec.type() + "'");
}
