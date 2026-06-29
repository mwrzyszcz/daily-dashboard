#include "NamedayWidget.h"

NamedayWidget::NamedayWidget(const std::string& id) noexcept
    : Widget(id)
{
}

void NamedayWidget::update(const DashboardState& state) noexcept
{
    last_ = state;
}

void NamedayWidget::render() const noexcept
{
}

