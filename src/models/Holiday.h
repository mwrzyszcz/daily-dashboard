#pragma once

#include <string>

struct Holiday
{
    bool isHoliday = false;
    std::string name;
    bool isPublicHoliday = false;  // true for official non-working days (urzędowo wolne dni)
};
