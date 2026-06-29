#pragma once

#include <string>

enum class WeatherIcon
{
    Unknown,
    ClearDay,
    ClearNight,
    FewClouds,
    ScatteredClouds,
    BrokenClouds,
    ShowerRain,
    Rain,
    Thunderstorm,
    Snow,
    Mist
};

inline std::string toString(WeatherIcon icon)
{
    switch (icon) {
    case WeatherIcon::ClearDay: return "01d";
    case WeatherIcon::ClearNight: return "01n";
    case WeatherIcon::FewClouds: return "02d";
    case WeatherIcon::ScatteredClouds: return "03d";
    case WeatherIcon::BrokenClouds: return "04d";
    case WeatherIcon::ShowerRain: return "09d";
    case WeatherIcon::Rain: return "10d";
    case WeatherIcon::Thunderstorm: return "11d";
    case WeatherIcon::Snow: return "13d";
    case WeatherIcon::Mist: return "50d";
    default: return "unknown";
    }
}
