#pragma once

#include <ostream>
#include <stdexcept>
#include <string>

// Base of all the errors of this project; the derived classes only change category().
class EbikeError : public std::runtime_error {
public:
    explicit EbikeError(const std::string& message) : std::runtime_error(message) {}

    virtual std::string category() const { return "ebike"; }

    friend std::ostream& operator<<(std::ostream& os, const EbikeError& error) {
        os << error.category() << " error: " << error.what();
        return os;
    }
};
