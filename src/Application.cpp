#include "Application.h"
#include "Trace.h"
#include <map>
#include <iostream>
#include <iomanip>

namespace ebike {

Application::Application(BikeConfig config)
    : config_(std::move(config)),
      bike_(config_),
      kernel_(config_.ticks("kernel.trace_capacity"), config_.ticks("kernel.watchdog_timeout")),
      duration_(config_.ticks("sim.duration")),
      tickHz_(config_.ticks("kernel.tick_hz")),
      traceLines_(config_.ticks("sim.report_trace_lines")) {
    registerFromConfig();
}

std::function<void()> Application::bodyFor(const std::string& type) {
    const std::map<std::string, std::function<void()>> bodies = {
        {"sensor_poll", [this] { bike_.pollSensors(); }},
        {"brake_monitor", [this] { bike_.checkBrake(); }},
        {"motor_control", [this] { bike_.runMotorControl(); }},
        {"battery_monitor",
         [this] {
             if (const std::optional<std::string> alert = bike_.monitorBattery()) {
                 kernel_.note(TraceKind::Alert, *alert);
             }
         }},
        {"display_update", [this] { bike_.refreshDisplay(); }},
        {"telemetry", [this] { bike_.logTelemetry(); }},
    };
    const auto found = bodies.find(type);
    if (found == bodies.end()) {
        throw ConfigError("unknown task type '" + type + "'");
    }
    return found->second;
}

std::function<void()> Application::actionFor(const std::string& action) {
    const std::map<std::string, std::function<void()>> actions = {
        {"cycle_assist", [this] { bike_.cycleAssist(); }},
        {"feed_watchdog", [this] { kernel_.feedWatchdogIfHealthy(); }},
    };
    const auto found = actions.find(action);
    if (found == actions.end()) {
        throw ConfigError("unknown timer action '" + action + "'");
    }
    return found->second;
}

void Application::registerFromConfig() {
    if (config_.tasks().empty()) {
        throw ConfigError("the config defines no tasks");
    }
    if (config_.totalUtilization() > 1.0) {
        throw ConfigError("task set is not schedulable: utilisation above 100%");
    }
    for (const TaskSpec& spec : config_.tasks()) {
        kernel_.addTask(spec, bodyFor(spec.type()));
    }
    for (const TimerSpec& spec : config_.timers()) {
        kernel_.addTimer(spec, actionFor(spec.action()));
    }
}

void Application::printTaskTable(std::ostream& out) const {
    out << std::left << std::setw(16) << "task" << std::right << std::setw(4) << "pri" << std::setw(8) << "jobs"
        << std::setw(8) << "done" << std::setw(8) << "missed" << std::setw(9) << "overrun" << std::setw(9) << "max-resp"
        << std::setw(8) << "bus" << "\n";
    for (const auto& task : kernel_.scheduler().tasks()) {
        out << *task << "\n";
    }
}

void Application::printTrace(std::ostream& out) const {
    const TraceBuffer& trace = kernel_.trace();
    out << "\nTrace: " << trace << "\n";
    out << "  switches=" << trace.countOf(TraceKind::Switch) << " blocks=" << trace.countOf(TraceKind::Block)
        << " unblocks=" << trace.countOf(TraceKind::Unblock) << " misses=" << trace.countOf(TraceKind::DeadlineMiss)
        << " overruns=" << trace.countOf(TraceKind::Overrun) << " alerts=" << trace.countOf(TraceKind::Alert)
        << " watchdog=" << trace.countOf(TraceKind::WatchdogReset) << "\n";
    for (const TraceEvent& event : trace.tail(traceLines_)) {
        out << "  " << event << "\n";
    }
}

void Application::printRide(std::ostream& out) const {
    out << "\nRide: " << bike_.stats() << "\n";
    out << "  " << bike_.battery() << "\n";
    out << "  brake cut-offs=" << bike_.brakeCuts() << ", dropped samples=" << bike_.droppedSamples()
        << ", safe state=" << (bike_.inSafeState() ? "yes" : "no") << "\n";
    for (const std::string& alert : bike_.alertHistory()) {
        out << "  alert: " << alert << "\n";
    }
    out << "  " << bike_.display() << "\n";
    if (!bike_.telemetry().empty()) {
        out << "  last telemetry: " << bike_.telemetry().back() << "\n";
    }
}

void Application::printReport(std::ostream& out) const {
    const Scheduler& scheduler = kernel_.scheduler();
    out << "=== E-bike RTOS report ===\n";
    out << config_ << "\n";
    out << "Simulated " << kernel_.now() << " ticks at " << tickHz_ << " Hz, CPU load "
        << Format::percent(scheduler.cpuUtilization()) << " (idle " << scheduler.idleTicks()
        << " ticks), context switches " << scheduler.contextSwitches() << "\n\n";
    printTaskTable(out);
    out << "\n" << scheduler.bus() << "\n";
    out << kernel_.timers() << "\n";
    out << kernel_.watchdog() << "\n";
    printRide(out);
    printTrace(out);
}

int Application::run(std::ostream& out) {
    kernel_.run(
        duration_, [this](Tick now) { bike_.physicsStep(now); }, [this] { bike_.enterSafeState(); });
    printReport(out);
    return kernel_.scheduler().totalDeadlineMisses() == 0 ? 0 : 2;
}

std::ostream& operator<<(std::ostream& os, const Application& app) {
    return os << "Application{" << app.config_ << ", " << app.kernel_ << "}";
}

}
