#include "widgets/WeatherWidget.h"
#include "Logger.h"
#include <Arduino.h>

WeatherWidget::WeatherWidget(const std::string& id) noexcept
    : Widget(id)
{
    setPreferredSize(Size{800, 100});
}

void WeatherWidget::update(const DashboardState& state) noexcept
{
    temperature_ = std::to_string(state.weather.temperature) + " °C";
    description_ = state.weather.description;
    icon_ = toString(state.weather.icon);
}

void WeatherWidget::render() const noexcept
{
    char buf[256];
    std::snprintf(buf, sizeof(buf), "%s | %s | icon: %s",
                 temperature_.c_str(), description_.c_str(), icon_.c_str());
    Logger::info("WeatherWidget", buf);
}
