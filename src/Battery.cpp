#include "Battery.hpp"

#include <iomanip>
#include <ostream>

#include "ConfigError.hpp"

Battery::Battery(int cells, double capacityMah, double cellFullV, double cellEmptyV, double initialCharge)
    : cells_(cells), capacityAh_(capacityMah / 1000), cellFullV_(cellFullV), cellEmptyV_(cellEmptyV) {
    if (cells <= 0 || capacityMah <= 0 || cellFullV <= cellEmptyV || initialCharge < 0 || initialCharge > 1) {
        throw ConfigError("invalid battery parameters");
    }
    chargeAh_ = capacityAh_ * initialCharge;
}

double Battery::voltage() const {
    const double cellVoltage = cellEmptyV_ + (cellFullV_ - cellEmptyV_) * chargeLevel();
    return cellVoltage * cells_;
}

void Battery::discharge(double powerW, double seconds) {
    const double currentA = powerW / voltage();
    chargeAh_ = chargeAh_ - currentA * seconds / 3600;
    if (chargeAh_ < 0) {
        chargeAh_ = 0;
    }
}

std::ostream& operator<<(std::ostream& os, const Battery& battery) {
    os << "Battery{" << std::fixed << std::setprecision(1) << battery.chargeLevel() * 100 << "%, "
       << battery.voltage() << " V}";
    return os;
}
