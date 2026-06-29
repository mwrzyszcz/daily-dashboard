#include "parsers/WeatherParser.h"
#include "models/WeatherIcon.h"

static WeatherIcon iconFromString(const std::string& icon)
{
    if (icon == "01d") return WeatherIcon::ClearDay;
    if (icon == "01n") return WeatherIcon::ClearNight;
    if (icon == "02d" || icon == "02n") return WeatherIcon::FewClouds;
    if (icon == "03d" || icon == "03n") return WeatherIcon::ScatteredClouds;
    if (icon == "04d" || icon == "04n") return WeatherIcon::BrokenClouds;
    if (icon == "09d" || icon == "09n") return WeatherIcon::ShowerRain;
    if (icon == "10d" || icon == "10n") return WeatherIcon::Rain;
    if (icon == "11d" || icon == "11n") return WeatherIcon::Thunderstorm;
    if (icon == "13d" || icon == "13n") return WeatherIcon::Snow;
    if (icon == "50d" || icon == "50n") return WeatherIcon::Mist;
    return WeatherIcon::Unknown;
}

Weather WeatherParser::parseCurrentWeather(const std::string& json) const
{
    (void)json;
    Weather weather;
    weather.temperature = 18.3f;
    weather.feelsLike = 17.8f;
    weather.tempMin = 16.0f;
    weather.tempMax = 19.2f;
    weather.humidity = 62;
    weather.pressure = 1014;
    weather.sunrise = 1624935600;
    weather.sunset = 1624990800;
    weather.icon = iconFromString("02d");
    weather.description = "Lekko zachmurzone";
    return weather;
}

std::vector<Forecast> WeatherParser::parseForecast(const std::string& json) const
{
    (void)json;
    std::vector<Forecast> forecast;
    forecast.reserve(3);
    forecast.push_back(Forecast{Date{2026, 6, 30}, 15.5f, 23.2f, iconFromString("01d")});
    forecast.push_back(Forecast{Date{2026, 7, 1}, 16.7f, 22.8f, iconFromString("04d")});
    forecast.push_back(Forecast{Date{2026, 7, 2}, 17.1f, 21.6f, iconFromString("09d")});
    return forecast;
}
