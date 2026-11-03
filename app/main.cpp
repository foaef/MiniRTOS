/*
 * Small e-bike RTOS simulation: a fixed-priority scheduler runs periodic tasks
 * (read sensors, control the motor, check the brake, check the battery) on a
 * simulated e-bike. 1 tick = 1 ms. Everything is read from assets/ebike_config.txt.
 *
 * stdin chooses what is printed: config, halfway, end, analysis (nothing valid = all three).
 */

#include <filesystem>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include "Config.hpp"
#include "ConfigLoader.hpp"
#include "Dashboard.hpp"
#include "EBike.hpp"
#include "EbikeError.hpp"
#include "InputError.hpp"
#include "Scheduler.hpp"
#include "SchedulabilityAnalysis.hpp"

namespace {

// Reads all of stdin (broke windows CI before because pipe problems), reporting wrong words.
std::set<std::string> readCommands() {
    std::set<std::string> commands;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream words(line);
        std::string command;
        if (!(words >> command)) {
            continue;
        }
        try {
            if (command != "config" && command != "halfway" && command != "end" && command != "analysis") {
                throw InputError("unknown command '" + command + "' (use: config, halfway, end, analysis)");
            }
            commands.insert(command);
        } catch (const InputError& error) {
            std::cerr << error << "\n";
        }
    }
    return commands;
}

}  // namespace

int main(int argc, char** argv) {
    try {
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
        const Dashboard dashboard(config.whole("dashboard.bar_width"));
        Scheduler halfway = scheduler;  // copy constructor

        const int duration = config.whole("sim.duration");
        for (int now = 0; now < duration; now++) {
            bike.physicsStep(now);
            scheduler.tick(bike);
            if (now == duration / 2) {
                halfway = scheduler;  // copy assignment
            }
        }

        const std::set<std::string> commands = readCommands();
        const bool all = commands.empty();
        if (all || commands.count("config") > 0) {
            std::cout << loader << "\n" << config << "\n\n";
        }
        if (all || commands.count("halfway") > 0) {
            std::cout << "At the halfway point:\n" << halfway << "\n\n";
        }
        if (all || commands.count("end") > 0) {
            std::cout << "At the end:\n" << scheduler << "\n" << bike << "\n\n"
                      << dashboard << "\n" << dashboard.render(bike.speedKmh(), bike.chargeLevel(), bike.motorPowerW())
                      << "\n";
        }
        if (all || commands.count("analysis") > 0) {
            const SchedulabilityAnalysis analysis(scheduler.tasks());
            std::cout << "Timing analysis (on paper, then checked against the simulation):\n" << analysis << "\n\n";
        }
        if (scheduler.totalMisses() > 0) {
            return 2;
        }
        return 0;
    } catch (const EbikeError& error) {
        std::cerr << error << "\n";
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << "\n";
        return 1;
    }
}
