#pragma once

#include "widgets/Widget.h"
#include "models/DashboardState.h"

class WeatherWidget : public Widget
{
public:
    explicit WeatherWidget(const std::string& id) noexcept;
    ~WeatherWidget() override = default;

    void update(const DashboardState& state) noexcept override;
    void render() const noexcept override;

private:
    std::string temperature_;
    std::string description_;
    std::string icon_;
};
