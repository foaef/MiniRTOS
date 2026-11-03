#pragma once

#include <iosfwd>

// Holding the brake down counts as one press.
class Brake {
    double decelerationMs2_;
    bool wasPressed_ = false;
    int presses_ = 0;

public:
    explicit Brake(double decelerationMs2);

    double deceleration() const { return decelerationMs2_; }

    void update(bool pressed);

    friend std::ostream& operator<<(std::ostream& os, const Brake& brake);
};
