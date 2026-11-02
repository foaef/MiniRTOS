#pragma once

#include <iosfwd>

// Battery pack: the voltage goes down as the charge goes down.
class Battery {
    int cells_;
    double capacityAh_;
    double cellFullV_;
    double cellEmptyV_;
    double chargeAh_;  // charge left

public:
    Battery(int cells, double capacityMah, double cellFullV, double cellEmptyV, double initialCharge);

    // Charge level from 0.0 (empty) to 1.0 (full).
    double chargeLevel() const { return chargeAh_ / capacityAh_; }

    double voltage() const;

    // Take energy out of the battery: current = power / voltage.
    void discharge(double powerW, double seconds);

    friend std::ostream& operator<<(std::ostream& os, const Battery& battery);
};
