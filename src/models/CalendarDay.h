#pragma once

#include <optional>
#include <string>
#include <vector>
#include "Holiday.h"

struct CalendarDay
{
    uint8_t day = 0;
    bool inMonth = false;
    bool isToday = false;
    bool isWeekend = false;
    std::vector<std::string> namedays;
    Holiday holiday;  // Now includes isPublicHoliday flag
};
