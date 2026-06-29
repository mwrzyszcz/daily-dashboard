#pragma once

#include <string>
#include <cstdint>
#include "WeatherIcon.h"
#include "Date.h"

/**
 * @brief Reprezentuje prognozę pogody dla pojedynczego dnia.
 */
class Forecast
{
public:
    Forecast() = default;
    Forecast(const Date& dateValue, float minTemperature, float maxTemperature, WeatherIcon weatherIcon) noexcept
        : date(dateValue)
        , minTemp(minTemperature)
        , maxTemp(maxTemperature)
        , icon(weatherIcon)
    {
    }

    Date date;
    float minTemp = 0.0f;
    float maxTemp = 0.0f;
    WeatherIcon icon = WeatherIcon::Unknown;
};
