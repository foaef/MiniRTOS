#ifndef APPLICATION_H
#define APPLICATION_H

#include "Common.h"
#include "Config.h"
#include "EBike.h"
#include "Kernel.h"
#include <functional>
#include <ostream>

namespace ebike {

class Application {
    BikeConfig config_;
    EBike bike_;
    Kernel kernel_;
    Tick duration_;
    Tick tickHz_;
    std::size_t traceLines_;

    std::function<void()> bodyFor(const std::string& type);
    std::function<void()> actionFor(const std::string& action);
    void registerFromConfig();
    void printTaskTable(std::ostream& out) const;
    void printTrace(std::ostream& out) const;
    void printRide(std::ostream& out) const;
    void printReport(std::ostream& out) const;

public:
    explicit Application(BikeConfig config);
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    ~Application() = default;
    
    int run(std::ostream& out);

    friend std::ostream& operator<<(std::ostream& os, const Application& app);
};

}

#endif
