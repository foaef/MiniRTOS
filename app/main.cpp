// MiniRTOS: a fixed-priority real-time scheduler running e-bike firmware tasks.
// On the PC the bike is simulated (SimHal); on the Pico 2 a PicoHal will take its place.
//
// Data files: assets/ebike_config.txt (bike, battery, assist, tasks) and the ride file it names.
// 1 tick = 1 ms.

#include <filesystem>
#include <iostream>
#include <string>

#include "Config.hpp"
#include "Errors.hpp"
#include "Scheduler.hpp"
#include "SimHal.hpp"
#include "TaskFactory.hpp"
#include "VehicleState.hpp"

int main(int argc, char** argv) {
    try {
        const std::string path = argc > 1 ? argv[1] : "assets/ebike_config.txt";

        const Config config(path);
        std::cout << config << "\n\n";

        SimHal bike(config);
        VehicleState state(bike.wheelSpeedKmh(), 100);
        Scheduler scheduler(config.whole("trace.capacity"));
        for (const TaskSpec& spec : config.tasks()) {
            scheduler.addTask(makeTask(spec, config));
        }

        // Check on paper before running: can every task always meet its deadline?
        std::cout << "Response time analysis: the task set is "
                  << (scheduler.isSchedulable() ? "schedulable" : "NOT schedulable") << " (bounds:";
        for (const int bound : scheduler.responseTimeBounds()) {
            std::cout << ' ' << bound;
        }
        std::cout << " ticks)\n\n";

        // Run the ride; keep a snapshot of the scheduler halfway (copy and assignment).
        const int duration = config.whole("sim.duration_ms");
        Scheduler halfway = scheduler;
        for (int now = 0; now < duration; now++) {
            bike.step(now);               // the world moves
            scheduler.tick(bike, state);  // the CPU runs one task for one tick
            if (now == duration / 2) {
                halfway = scheduler;
            }
        }

        std::cout << "Halfway:\n" << halfway << "\n\nEnd of the ride:\n" << scheduler << "\n\n";
        std::cout << state << "\n" << bike << "\n";
        std::cout << "\nSafety: worst time from brake press to motor off = " << bike.worstBrakeReactionMs()
                  << " ms, missed deadlines = " << scheduler.totalMisses() << "\n";
        return scheduler.totalMisses() == 0 ? 0 : 2;
    } catch (const ConfigError& error) {
        std::cerr << "config error: " << error.what() << "\n";
        return 1;
    } catch (const SchedulingError& error) {
        std::cerr << "scheduling error: " << error.what() << "\n";
        return 1;
    } catch (const HardwareError& error) {
        // a firmware bug: on the real bike this is where the watchdog would cut the motor
        std::cerr << "hardware error: " << error.what() << "\n";
        return 3;
    }
}
