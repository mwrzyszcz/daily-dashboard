#pragma once

#include <string>
#include "Configuration.h"

class WeatherClient
{
public:
    explicit WeatherClient(const Configuration& configuration) noexcept;
    ~WeatherClient() = default;

    std::string getCurrentWeatherJson() const;
    std::string getForecastJson() const;

private:
    const Configuration& configuration_;
};
