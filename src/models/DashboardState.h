#pragma once

#include <vector>
#include <string>
#include "Date.h"
#include "Time.h"
#include "Weather.h"
#include "Forecast.h"
#include "Holiday.h"
#include "models/CalendarMonth.h"

/**
 * @brief Zawiera wszystkie dane potrzebne do renderowania pulpitu.
 */
struct DashboardState
{
    Date date;
    Time time;
    Weather weather;
    std::vector<Forecast> forecast;

    CalendarMonth calendarMonth;
    std::vector<std::string> namedays;
    Holiday holiday;  // Now includes isPublicHoliday flag
};
