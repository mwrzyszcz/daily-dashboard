#pragma once

#include <string>
#include "widgets/Widget.h"
#include "models/DashboardState.h"

class ForecastWidget : public Widget
{
public:
    explicit ForecastWidget(const std::string& id) noexcept;
    ~ForecastWidget() override = default;

    void update(const DashboardState& state) noexcept override;
    void render() const noexcept override;

private:
    std::string forecastSummary_;
};
