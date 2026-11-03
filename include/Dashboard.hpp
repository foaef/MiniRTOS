#pragma once

#include <iosfwd>
#include <string>

class Dashboard {
    int barWidth_;

public:
    explicit Dashboard(int barWidth);

    std::string batteryBar(double charge) const;

    std::string render(double speedKmh, double charge, double motorPowerW) const;

    friend std::ostream& operator<<(std::ostream& os, const Dashboard& dashboard);
};
