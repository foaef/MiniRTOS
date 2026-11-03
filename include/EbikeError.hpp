#pragma once

#include <ostream>
#include <stdexcept>
#include <string>

// Base of all the errors of this project. The derived classes only change the
// category, so one catch block can handle every kind of error and still print
// the right label (operator<< calls the virtual category()).
class EbikeError : public std::runtime_error {
public:
    explicit EbikeError(const std::string& message) : std::runtime_error(message) {}

    virtual std::string category() const { return "ebike"; }

    friend std::ostream& operator<<(std::ostream& os, const EbikeError& error) {
        os << error.category() << " error: " << error.what();
        return os;
    }
};
