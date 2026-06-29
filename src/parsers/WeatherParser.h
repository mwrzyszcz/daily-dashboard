#pragma once

#include <string>
#include <vector>
#include "models/Weather.h"
#include "models/Forecast.h"

class WeatherParser
{
public:
    WeatherParser() = default;
    ~WeatherParser() = default;

    Weather parseCurrentWeather(const std::string& json) const;
    std::vector<Forecast> parseForecast(const std::string& json) const;
};
