#include "Logger.h"
#include "repositories/WeatherRepository.h"

WeatherRepository::WeatherRepository(const Configuration& configuration,
                                     WeatherClient& client,
                                     WeatherParser& parser) noexcept
    : configuration_(configuration)
    , client_(client)
    , parser_(parser)
{
}

Weather WeatherRepository::fetchCurrentWeather() const
{
    Logger::info("WeatherRepo", "Fetching current weather data");
    const std::string payload = client_.getCurrentWeatherJson();
    return parser_.parseCurrentWeather(payload);
}

std::vector<Forecast> WeatherRepository::fetchForecast() const
{
    Logger::info("WeatherRepo", "Fetching weather forecast data");
    const std::string payload = client_.getForecastJson();
    return parser_.parseForecast(payload);
}
