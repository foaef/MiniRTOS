#include "TaskAction.hpp"

#include <ostream>

std::ostream& operator<<(std::ostream& os, const TaskAction& action) {
    os << action.name();
    return os;
}
