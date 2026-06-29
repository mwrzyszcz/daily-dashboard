#pragma once

#include <string>
#include <cstdint>

class Time
{
public:
    Time() = default;
    Time(uint8_t hoursValue, uint8_t minutesValue) noexcept
        : hours(hoursValue)
        , minutes(minutesValue)
    {
    }

    uint8_t hours = 0;
    uint8_t minutes = 0;

    std::string toString() const;
};
