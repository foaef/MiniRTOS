#pragma once

#include <iosfwd>
#include <string>

// A list of text messages with a fixed capacity. The capacity comes from the
// config file, so the messages are stored in an array created with `new`.
// Because the class owns that memory, it needs the rule of three:
// copy constructor, operator= and destructor.
class EventLog {
    int capacity_;
    std::string* lines_;
    int count_ = 0;
    int dropped_ = 0;  // messages that did not fit

    // A new array with the same capacity and a copy of the stored messages.
    static std::string* copyLines(const EventLog& other);

public:
    explicit EventLog(int capacity);

    // Copy constructor: make a separate array and copy the messages into it.
    EventLog(const EventLog& other);

    // Assignment: build the copy first, then free our old array (nothing is lost if `new` fails).
    EventLog& operator=(const EventLog& other);

    // Destructor: give the array back.
    ~EventLog();

    void add(const std::string& text);

    friend std::ostream& operator<<(std::ostream& os, const EventLog& log);
};
