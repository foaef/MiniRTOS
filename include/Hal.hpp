#pragma once

#include <iosfwd>

// Hardware abstraction layer: the only way the firmware talks to the bike.
// SimHal implements it on the PC; on the Pico 2 a PicoHal will read GPIO/ADC
// and drive the motor PWM instead, and the tasks will not change.
class Hal {
    virtual void printStatus(std::ostream& os) const = 0;

public:
    Hal() = default;
    virtual ~Hal() = default;
    Hal(const Hal&) = delete;
    Hal& operator=(const Hal&) = delete;

    virtual bool brakePressed() const = 0;
    virtual double throttle() const = 0;  // 0..1
    virtual double wheelSpeedKmh() const = 0;
    virtual double packVoltage() const = 0;
    virtual void setMotorPower(double watts) = 0;

    friend std::ostream& operator<<(std::ostream& os, const Hal& hal);
};
