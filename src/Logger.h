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
        char ts[24];
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 0)) {
            std::snprintf(ts, sizeof(ts), "%02d-%02d-%04d %02d:%02d:%02d",
                          timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
                          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        } else {
            const unsigned long ms = millis();
            std::snprintf(ts, sizeof(ts), "%lu.%03lu", ms / 1000UL, ms % 1000UL);
        }
        Serial.print('[');
        Serial.print(ts);
        Serial.print("] [");
        Serial.print(level);
        Serial.print("] ");
        Serial.print(tag);
        Serial.print(": ");
        Serial.println(message);
    }
};

#endif // LOGGER_H
