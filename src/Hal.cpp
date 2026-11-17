#include "Hal.hpp"

#include <ostream>

std::ostream& operator<<(std::ostream& os, const Hal& hal) {
    hal.printStatus(os);
    return os;
}
