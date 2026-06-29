#include "services/WeatherService.h"

WeatherService::WeatherService(WeatherRepository& repository) noexcept
    : repository_(repository)
{
}

Weather WeatherService::getCurrentWeather() const
{
    return repository_.fetchCurrentWeather();
}

std::vector<Forecast> WeatherService::getForecast() const
{
    return repository_.fetchForecast();
}
