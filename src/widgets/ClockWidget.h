#pragma once

#include "Widget.h"

class ClockWidget : public Widget
{
public:
    ClockWidget(const std::string& id) noexcept;
    ~ClockWidget() override = default;

    void update(const DashboardState& state) noexcept override;
    void render() const noexcept override;

private:
    DashboardState last_{};
};

