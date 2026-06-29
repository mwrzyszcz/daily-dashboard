#pragma once

#include <vector>
#include "models/Weather.h"
#include "models/Forecast.h"

/**
 * @brief Interfejs serwisu pogodowego.
 */
class IWeatherService
{
public:
    virtual ~IWeatherService() = default;
    virtual Weather getCurrentWeather() const = 0;
    virtual std::vector<Forecast> getForecast() const = 0;
};
