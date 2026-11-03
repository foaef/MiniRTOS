#pragma once

#include <iosfwd>

class Battery {
    int cells_;
    double capacityAh_;
    double cellFullV_;
    double cellEmptyV_;
    double chargeAh_;

public:
    Battery(int cells, double capacityMah, double cellFullV, double cellEmptyV, double initialCharge);

    double chargeLevel() const { return chargeAh_ / capacityAh_; }

    double voltage() const;

    void discharge(double powerW, double seconds);

    friend std::ostream& operator<<(std::ostream& os, const Battery& battery);
};
