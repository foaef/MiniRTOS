#ifndef EBIKE_H
#define EBIKE_H

#include "Common.h"
#include "Config.h"
#include "Domain.h"
#include "StaticQueue.h"
#include <vector>
#include <optional>
#include <string>

namespace ebike {

enum class BatteryState { Normal, Low, Critical };

class EBike {
    double dtSeconds_;
    Battery battery_;
    MotorController motor_;
    BikeDynamics dynamics_;
    RiderProfile rider_;
    SensorHub sensors_;
    AssistTable assist_;
    AssistLevel level_;
    Display display_;
    RideStats stats_;
    StaticQueue<SensorSample, 8> samples_;
    StaticQueue<std::string, 4> alerts_;
    std::optional<SensorSample> latest_;
    std::vector<std::string> telemetry_;
    std::vector<std::string> alertHistory_;
    BatteryState batteryState_{BatteryState::Normal};
    double lowSoc_;
    double criticalSoc_;
    double lowBatteryFactor_;
    double defaultWhPerKm_;
    Tick alertDisplayTicks_;
    Tick clock_{0};
    Tick lastControl_{0};
    std::size_t brakeCuts_{0};
    bool braking_{false};
    bool safeMode_{false};

    double batteryFactor() const;
    static const char* stateName(BatteryState state);
    BatteryState classify(double soc) const;
    double rangeKm() const;
    static constexpr double kMinimumKmForConsumption = 0.05;

public:
    explicit EBike(const BikeConfig& cfg);
    
    void physicsStep(Tick now);
    void pollSensors();
    void checkBrake();
    void runMotorControl();
    std::optional<std::string> monitorBattery();
    void refreshDisplay();
    void logTelemetry();
    void cycleAssist();
    void enterSafeState();
    
    const RideStats& stats() const { return stats_; }
    const Battery& battery() const { return battery_; }
    const Display& display() const { return display_; }
    const std::vector<std::string>& telemetry() const { return telemetry_; }
    const std::vector<std::string>& alertHistory() const { return alertHistory_; }
    std::size_t brakeCuts() const { return brakeCuts_; }
    bool inSafeState() const { return safeMode_; }
    std::size_t droppedSamples() const { return samples_.rejected(); }

    friend std::ostream& operator<<(std::ostream& os, const EBike& bike);
};

}

#endif
