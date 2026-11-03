#include "Dashboard.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <ostream>
#include <sstream>

#include "ConfigError.hpp"

Dashboard::Dashboard(int barWidth) : barWidth_(barWidth) {
    if (barWidth_ <= 0) {
        throw ConfigError("dashboard.bar_width must be positive");
    }
}

std::string Dashboard::batteryBar(double charge) const {
    const double level = std::max(0.0, std::min(1.0, charge));
    const int filled = static_cast<int>(std::lround(level * barWidth_));
    return "[" + std::string(static_cast<std::size_t>(filled), '#') +
           std::string(static_cast<std::size_t>(barWidth_ - filled), '-') + "]";
}

std::string Dashboard::render(double speedKmh, double charge, double motorPowerW) const {
    std::ostringstream line;
    line << std::fixed << std::setprecision(1) << "Speed " << speedKmh << " km/h | Battery " << batteryBar(charge)
         << " " << std::setprecision(0) << charge * 100 << "% | Motor " << motorPowerW << " W";
    return line.str();
}

std::ostream& operator<<(std::ostream& os, const Dashboard& dashboard) {
    os << "Dashboard{bar width " << dashboard.barWidth_ << "}";
    return os;
}
