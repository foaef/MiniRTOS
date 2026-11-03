#pragma once

#include <iosfwd>

#include "Battery.hpp"
#include "Config.hpp"
#include "Motor.hpp"

// The whole bike. It owns a Battery and a Motor, simulates the physics, and
// knows what each type of task has to do.
class EBike {
    // physics constants
    static constexpr double GRAVITY = 9.81;
    static constexpr double AIR_DENSITY = 1.2;

    // values from the config file
    double secondsPerTick_;
    Battery battery_;
    Motor motor_;
    double massKg_;
    double rollingResistance_;
    double dragArea_;
    double brakeDecel_;      // how fast the bike slows down while braking (m/s^2)
    double riderPowerW_;
    double assistRatio_;
    double lowBatteryLevel_;
    double lowBatteryFactor_;
    int brakeEvery_;         // the rider presses the brake every this many ticks...
    int brakeDuration_;      // ...for this many ticks

    // state of the bike
    double speedMs_;
    double distanceKm_ = 0;
    int now_ = 0;
    bool lowBattery_ = false;
    bool wasBraking_ = false;
    int brakeCuts_ = 0;

    // what the sensors last measured (a copy, it can be a few ticks old)
    double sensedSpeedKmh_ = 0;
    bool sensedBrake_ = false;

    // Is the rider pressing the brake at this tick?
    bool brakePressedAt(int tick) const;

    // Length of one tick in seconds (the config gives ticks per second).
    static double tickLength(const Config& cfg);

public:
    explicit EBike(const Config& cfg);

    // --- the four things a task can do (called by the TaskAction classes) ---
    void readSensors();    // copy the real speed and brake state into the "sensed" values
    void controlMotor();   // decide the motor power using only the sensed values
    void checkBrake();     // look at the brake directly and cut the motor at once
    void checkBattery();   // remember if the battery is low

    // The simulated world, called once per tick (it is NOT a task).
    void physicsStep(int now);

    friend std::ostream& operator<<(std::ostream& os, const EBike& bike);
};
