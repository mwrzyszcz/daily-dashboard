#pragma once

#include <string>
#include <cstdint>

/**
 * @brief Prostym model daty z dodatkowymi polami pomocnymi w kalendarzu.
 */
class Date
{
public:
    Date() = default;
    Date(uint16_t yearValue, uint8_t monthValue, uint8_t dayValue) noexcept
        : year(yearValue)
        , month(monthValue)
        , day(dayValue)
    {
    }

    uint16_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;
    /** 1 = Monday .. 7 = Sunday */
    uint8_t weekday = 0;
    uint16_t dayOfYear = 0;
    uint8_t weekNumber = 0;

    std::string toString() const;
};
