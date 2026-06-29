#include "widgets/ForecastWidget.h"
#include "Logger.h"
#include <Arduino.h>

ForecastWidget::ForecastWidget(const std::string& id) noexcept
    : Widget(id)
{
    setPreferredSize(Size{800, 120});
}

void ForecastWidget::update(const DashboardState& state) noexcept
{
    forecastSummary_.clear();
    for (const auto& forecast : state.forecast) {
        if (!forecastSummary_.empty()) {
            forecastSummary_ += " | ";
        }
        forecastSummary_ += forecast.date.toString();
        forecastSummary_ += ": ";
        forecastSummary_ += std::to_string(forecast.minTemp);
        forecastSummary_ += "-";
        forecastSummary_ += std::to_string(forecast.maxTemp);
        forecastSummary_ += "°C ";
        forecastSummary_ += toString(forecast.icon);
    }
}

void ForecastWidget::render() const noexcept
{
    std::string msg = "[ForecastWidget] " + forecastSummary_;
    Logger::info("ForecastWidget", msg.c_str());
}
