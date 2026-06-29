#include "CalendarWidget.h"

CalendarWidget::CalendarWidget(const std::string& id) noexcept
    : Widget(id)
{
}

void CalendarWidget::update(const DashboardState& state) noexcept
{
    last_ = state;
}

void CalendarWidget::render() const noexcept
{
}

