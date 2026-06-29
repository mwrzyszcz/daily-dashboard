#include "Date.h"
#include <cstdio>
#include <ctime>

static uint8_t weekdayFromTm(const std::tm& t)
{
    // tm_wday: 0 = Sunday, 1 = Monday .. 6 = Saturday
    // We want 1 = Monday .. 7 = Sunday
    if (t.tm_wday == 0) {
        return 7;
    }
    return static_cast<uint8_t>(t.tm_wday);
}

static uint16_t dayOfYearFromTm(const std::tm& t)
{
    return static_cast<uint16_t>(t.tm_yday + 1);
}

static uint8_t isoWeekNumber(const std::tm& t)
{
    // Compute ISO week number using tm
    std::tm tmp = t;
    char buf[8] = {};
    std::strftime(buf, sizeof(buf), "%V", &tmp);
    return static_cast<uint8_t>(std::atoi(buf));
}

std::string Date::toString() const
{
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%04u-%02u-%02u", year, month, day);
    return std::string(buffer);
}
