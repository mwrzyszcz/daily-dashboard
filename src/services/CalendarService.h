#pragma once

#include "models/CalendarMonth.h"
#include "models/Date.h"

class CalendarService
{
public:
    CalendarService() = default;
    ~CalendarService() = default;

    CalendarMonth getCurrentMonth(const Date& date) const noexcept;

    static uint8_t daysInMonth(uint16_t year, uint8_t month) noexcept;
    static uint8_t firstWeekday(uint16_t year, uint8_t month) noexcept; // 1 = Monday .. 7 = Sunday
    static uint8_t weekNumber(const Date& date) noexcept;
    static bool isWeekend(uint8_t weekday) noexcept; // weekday: 1..7
    static bool isToday(const Date& a, const Date& b) noexcept;
};
