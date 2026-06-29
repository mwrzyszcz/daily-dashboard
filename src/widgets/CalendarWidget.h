#pragma once

#include "Widget.h"

class CalendarWidget : public Widget
{
public:
    CalendarWidget(const std::string& id) noexcept;
    ~CalendarWidget() override = default;

    void update(const DashboardState& state) noexcept override;
    void render() const noexcept override;

private:
    DashboardState last_{};
};

