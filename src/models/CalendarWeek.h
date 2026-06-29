#pragma once

#include <array>
#include "CalendarDay.h"

struct CalendarWeek
{
    std::array<CalendarDay, 7> days; // 0 = Monday .. 6 = Sunday
};
