#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <string>

class Logger {
public:
    static void begin(unsigned long baud = 115200) noexcept
    {
        Serial.begin(baud);
    }

    static void info(const char* tag, const char* message) noexcept
    {
        write("INFO", tag, message);
    }

    static void warn(const char* tag, const char* message) noexcept
    {
        write("WARN", tag, message);
    }

    static void error(const char* tag, const char* message) noexcept
    {
        write("ERROR", tag, message);
    }

    static void info(const char* tag, const std::string& message) noexcept
    {
        info(tag, message.c_str());
    }

    static void warn(const char* tag, const std::string& message) noexcept
    {
        warn(tag, message.c_str());
    }

    static void error(const char* tag, const std::string& message) noexcept
    {
        error(tag, message.c_str());
    }

private:
    static void write(const char* level, const char* tag, const char* message) noexcept
    {
        Serial.print('[');
        Serial.print(level);
        Serial.print("] ");
        Serial.print(tag);
        Serial.print(": ");
        Serial.println(message);
    }
};

#endif // LOGGER_H
