#include "HolidayWidget.h"

HolidayWidget::HolidayWidget(const std::string& id) noexcept
    : Widget(id)
{
}

void HolidayWidget::update(const DashboardState& state) noexcept
{
    last_ = state;
}

void HolidayWidget::render() const noexcept
{
}

