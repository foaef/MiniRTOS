#ifndef DOMAIN_H
#define DOMAIN_H

#include "Common.h"
#include "Config.h"
#include <cstdint>
#include <vector>
#include <string>
#include <optional>
#include <array>

namespace ebike {

class NoiseSource {
    std::uint64_t state_;

public:
    explicit NoiseSource(std::uint64_t seed);
    double next();
    friend std::ostream& operator<<(std::ostream& os, const NoiseSource& source);
};

class SensorSample {
    Tick tick_{0};
    double speedKmh_{0.0};
    double pedalPowerW_{0.0};
    double batteryV_{0.0};
    double currentA_{0.0};
    double motorTempC_{0.0};
    bool brake_{false};

public:
    SensorSample() = default;
    SensorSample(Tick tick, double speedKmh, double pedalPowerW, double batteryV, double currentA,
                 double motorTempC, bool brake);
    
    Tick tick() const { return tick_; }
    double speedKmh() const { return speedKmh_; }
    double pedalPowerW() const { return pedalPowerW_; }
    double batteryV() const { return batteryV_; }
    double currentA() const { return currentA_; }
    double motorTempC() const { return motorTempC_; }
    bool brake() const { return brake_; }

    friend std::ostream& operator<<(std::ostream& os, const SensorSample& s);
};

class PidController {
    double kp_;
    double ki_;
    double kd_;
    double outMin_;
    double outMax_;
    double integral_{0.0};
    double previousError_{0.0};
    bool primed_{false};

public:
    PidController(double kp, double ki, double kd, double outMin, double outMax);
    double update(double setpoint, double measured, double dtSeconds);
    void reset();
    friend std::ostream& operator<<(std::ostream& os, const PidController& pid);
};

class Battery {
    int cellsSeries_;
    double capacityAh_;
    double cellFullV_;
    double cellEmptyV_;
    double internalResistance_;
    double chargeAh_;

public:
    Battery(int cellsSeries, double capacityMah, double cellFullV, double cellEmptyV, double internalResistance,
            double initialSoc);
    double stateOfCharge() const { return chargeAh_ / capacityAh_; }
    double openCircuitVoltage() const;
    double terminalVoltage(double currentA) const;
    double remainingWh() const;
    double currentForPower(double powerW) const;
    void discharge(double currentA, double dtSeconds);
    double estimatedRangeKm(double whPerKm) const;

    friend std::ostream& operator<<(std::ostream& os, const Battery& battery);
};

class RiderProfile {
    static constexpr double kTwoPi = 6.283185307179586;
    double basePowerW_;
    double swingW_;
    Tick powerPeriod_;
    double gradientAmplitudePct_;
    Tick gradientPeriod_;
    Tick brakeEvery_;
    Tick brakeDuration_;
    
    static double wave(Tick now, Tick period);

public:
    RiderProfile(double basePowerW, double swingW, Tick powerPeriod, double gradientAmplitudePct,
                 Tick gradientPeriod, Tick brakeEvery, Tick brakeDuration);
    double pedalPowerAt(Tick now) const;
    double gradientAt(Tick now) const;
    bool brakingAt(Tick now) const;

    friend std::ostream& operator<<(std::ostream& os, const RiderProfile& rider);
};

class BikeDynamics {
    static constexpr double kGravity = 9.81;
    static constexpr double kAirDensity = 1.2;
    static constexpr double kMinSpeedMs = 1.0;
    double massKg_;
    double rollingResistance_;
    double dragArea_;
    double brakeDecel_;
    double speedMs_;
    double pedalPowerW_{0.0};
    double gradientPct_{0.0};
    bool braking_{false};

public:
    BikeDynamics(double massKg, double rollingResistance, double dragArea, double brakeDecel, double initialSpeedKmh);
    void setRiderInput(double pwr, double gradientPct, bool braking);
    void step(double dtSeconds, double motorPowerW);
    double speedKmh() const { return speedMs_ * 3.6; }
    double pedalPowerW() const { return pedalPowerW_; }

    friend std::ostream& operator<<(std::ostream& os, const BikeDynamics& bike);
};

enum class AssistLevel { Off, Eco, Tour, Sport, Turbo };

class AssistTable {
    static constexpr std::size_t kLevels = 5;
    std::array<double, kLevels> ratios_;

public:
    explicit AssistTable(const BikeConfig& config);
    double ratio(AssistLevel level) const { return ratios_[static_cast<std::size_t>(level)]; }
    static AssistLevel next(AssistLevel level);
    static AssistLevel fromIndex(Tick index);
    static const char* name(AssistLevel level);

    friend std::ostream& operator<<(std::ostream& os, const AssistTable& table);
};

class MotorController {
    PidController pid_;
    double maxPowerW_;
    double cutoffKmh_;
    double fadeKmh_;
    double derateStartC_;
    double limitC_;
    double heatRate_;
    double coolRate_;
    double responsePerS_;
    double ambientC_;
    double duty_{0.0};
    double powerW_{0.0};
    double tempC_;

public:
    MotorController(PidController pid, double maxPowerW, double cutoffKmh, double fadeKmh, double derateStartC,
                    double limitC, double heatRate, double coolRate, double responsePerS, double ambientC);
    double speedFactor(double speedKmh) const;
    double thermalFactor() const;
    double targetPower(const SensorSample& sample, double assistRatio, double batteryFactor) const;
    void regulate(double targetPowerW, double dtSeconds);
    void advance(double dtSeconds);
    void cutPower();
    double powerW() const { return powerW_; }
    double temperatureC() const { return tempC_; }

    friend std::ostream& operator<<(std::ostream& os, const MotorController& motor);
};

class SensorHub {
    NoiseSource noise_;
    double speedNoiseKmh_;
    double powerNoiseW_;
    double voltageNoiseV_;

public:
    SensorHub(std::uint64_t seed, double speedNoiseKmh, double powerNoiseW, double voltageNoiseV);
    SensorSample read(Tick now, const BikeDynamics& bike, const Battery& battery,
                      const MotorController& motor, const RiderProfile& rider);

    friend std::ostream& operator<<(std::ostream& os, const SensorHub& hub);
};

class Display {
    std::string topLine_;
    std::string bottomLine_;
    std::string alert_;
    Tick alertUntil_{0};
    std::size_t frames_{0};

public:
    void showAlert(const std::string& text, Tick until);
    void render(const SensorSample& sample, double soc, AssistLevel level, double rangeKm, Tick now);

    friend std::ostream& operator<<(std::ostream& os, const Display& display);
};

class RideStats {
    double distanceKm_{0.0};
    double elapsedS_{0.0};
    double maxSpeedKmh_{0.0};
    double batteryEnergyWh_{0.0};

public:
    void accumulate(double speedKmh, double batteryPowerW, double dtSeconds);
    double distanceKm() const { return distanceKm_; }
    double maxSpeedKmh() const { return maxSpeedKmh_; }
    double batteryEnergyWh() const { return batteryEnergyWh_; }
    double averageSpeedKmh() const;
    double whPerKm(double minimumKm) const;

    friend std::ostream& operator<<(std::ostream& os, const RideStats& stats);
};

}

#endif
