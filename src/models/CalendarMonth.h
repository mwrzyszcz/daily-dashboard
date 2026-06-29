#pragma once

#include <vector>
#include "CalendarWeek.h"

struct CalendarMonth
{
    uint16_t year = 0;
    uint8_t month = 0;
    std::vector<CalendarWeek> weeks;
};
