#pragma once

#include "Widget.h"

class HolidayWidget : public Widget
{
public:
    HolidayWidget(const std::string& id) noexcept;
    ~HolidayWidget() override = default;

    void update(const DashboardState& state) noexcept override;
    void render() const noexcept override;

private:
    DashboardState last_{};
};

