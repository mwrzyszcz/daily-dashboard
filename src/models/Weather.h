#pragma once

#include <string>
#include <cstdint>
#include "WeatherIcon.h"

/**
 * @brief Reprezentuje aktualne dane pogodowe.
 */
class Weather
{
public:
    float temperature = 0.0f;
    float feelsLike = 0.0f;
    float tempMin = 0.0f;
    float tempMax = 0.0f;
    uint8_t humidity = 0;
    uint16_t pressure = 0;
    float windSpeed = 0.0f;
    uint32_t sunrise = 0;
    uint32_t sunset = 0;
    WeatherIcon icon = WeatherIcon::Unknown;
    std::string description;
};