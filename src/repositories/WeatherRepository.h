#pragma once

#include <vector>
#include "Configuration.h"
#include "clients/WeatherClient.h"
#include "parsers/WeatherParser.h"
#include "models/Weather.h"
#include "models/Forecast.h"

class WeatherRepository
{
public:
    WeatherRepository(const Configuration& configuration,
                      WeatherClient& client,
                      WeatherParser& parser) noexcept;
    ~WeatherRepository() = default;

    Weather fetchCurrentWeather() const;
    std::vector<Forecast> fetchForecast() const;

private:
    const Configuration& configuration_;
    WeatherClient& client_;
    WeatherParser& parser_;
};
