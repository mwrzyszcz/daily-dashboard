#include "Time.h"
#include <cstdio>

std::string Time::toString() const
{
    char buffer[8];
    std::snprintf(buffer, sizeof(buffer), "%02u:%02u", hours, minutes);
    return std::string(buffer);
}
