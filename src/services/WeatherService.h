#pragma once

#include <vector>
#include "interfaces/IWeatherService.h"
#include "models/Weather.h"
#include "models/Forecast.h"
#include "repositories/WeatherRepository.h"

class WeatherService : public IWeatherService
{
public:
    explicit WeatherService(WeatherRepository& repository) noexcept;
    ~WeatherService() override = default;

    Weather getCurrentWeather() const override;
    std::vector<Forecast> getForecast() const override;

private:
    WeatherRepository& repository_;
};
