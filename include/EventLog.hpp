#pragma once

#include <iosfwd>
#include <string>

// Messages are stored in an array created with `new`, so the class needs the
// rule of three: copy constructor, operator= and destructor.
class EventLog {
    int capacity_;
    std::string* lines_;
    int count_ = 0;
    int dropped_ = 0;

    static std::string* copyLines(const EventLog& other);

public:
    explicit EventLog(int capacity);

    EventLog(const EventLog& other);

    // Assignment: build the copy first, then free our old array (nothing is lost if `new` fails).
    EventLog& operator=(const EventLog& other);

    ~EventLog();

    void add(const std::string& text);

    friend std::ostream& operator<<(std::ostream& os, const EventLog& log);
};
