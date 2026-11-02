/*
 * main.cpp - small e-bike RTOS simulation.
 *
 * A tiny scheduler runs periodic tasks (read sensors, control the motor,
 * check the brake, check the battery) on top of a simulated e-bike.
 *
 * Time is counted in ticks, 1 tick = 1 millisecond. Every tick the world
 * moves a little (EBike::physicsStep) and the scheduler lets one task use
 * the CPU for that tick (Scheduler::tick). Higher priority number = more
 * important task.
 *
 * Everything (bike parameters, task list) is read from assets/ebike_config.txt.
 */

#include <filesystem>
#include <iostream>
#include <string>

#include "Config.hpp"
#include "ConfigError.hpp"
#include "ConfigLoader.hpp"
#include "EBike.hpp"
#include "Scheduler.hpp"

int main(int argc, char** argv) {
    try {
        // the config file path can be given as the first argument
        std::string path = "assets/ebike_config.txt";
        if (argc > 1) {
            path = argv[1];
        } else if (!std::filesystem::exists(path)) {
            path = "../assets/ebike_config.txt";
        }

        const ConfigLoader loader(path);
        const Config config = loader.load();
        EBike bike(config);
        Scheduler scheduler(config.tasks(), config.whole("log.capacity"));
        Scheduler halfway = scheduler;  // copy constructor: a snapshot of the scheduler before the start

        // The simulation: each loop is one tick (1 ms).
        const int duration = config.whole("sim.duration");
        for (int now = 0; now < duration; now++) {
            bike.physicsStep(now);  // the world moves
            scheduler.tick(bike);   // the CPU runs one task for one tick
            if (now == duration / 2) {
                halfway = scheduler;  // copy assignment: the snapshot is refreshed halfway through
            }
        }

        std::cout << loader << "\n" << config << "\n\n";
        std::cout << "At the halfway point:\n" << halfway << "\n\n";
        std::cout << "At the end:\n" << scheduler << "\n" << bike << "\n";
        if (scheduler.totalMisses() > 0) {
            return 2;  // at least one task missed a deadline
        }
        return 0;
    } catch (const ConfigError& error) {
        std::cerr << error << "\n";
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << "\n";
        return 1;
    }
}
