#pragma once

#include <iosfwd>

#include "Config.hpp"
#include "Hal.hpp"
#include "Ride.hpp"

// The simulated bike: rider, road, physics and battery, driven by a recorded ride.
// The ride can also break a sensor on purpose. It measures how safely the firmware behaved (motor still pushing while braking).
class SimHal final : public Hal {
    Ride ride_;
    double massKg_;
    double rollingResistance_;
    double dragAreaM2_;
    double brakeDecelMs2_;
    double ratedPowerW_;
    int cells_;
    double cellEmptyV_;
    double cellFullV_;
    double capacityAh_;
    double internalResistanceOhm_;
    double throttleMinV_;
    double throttleMaxV_;

    double chargeAh_;
    double speedMs_;
    double motorPowerW_ = 0;
    bool brake_ = false;
    double throttle_ = 0;
    double cadenceRpm_ = 0;
    SensorFault fault_ = SensorFault::NONE;

    double distanceKm_ = 0;
    double maxSpeedKmh_ = 0;
    double motorEnergyWh_ = 0;
    int unsafeTicks_ = 0;     // ticks with the brake held and the motor still pushing
    int unsafeRun_ = 0;       // length of the current such run
    int worstUnsafeRun_ = 0;  // longest such run: the worst brake reaction time

    void printStatus(std::ostream& os) const override;

    // Voltage with no current flowing; it only depends on how charged the pack is.
    double openCircuitVoltage() const;

public:
    explicit SimHal(const Config& config);

    // Moves the world forward by one tick (1 ms), using the ride step active at `nowMs`.
    void step(int nowMs);

    int worstBrakeReactionMs() const { return worstUnsafeRun_; }

    bool brakePressed() const override { return brake_; }
    double throttleVoltage() const override;
    double pedalCadenceRpm() const override { return cadenceRpm_; }
    double wheelSpeedKmh() const override;
    double packVoltage() const override;
    void setMotorPower(double watts) override;
};