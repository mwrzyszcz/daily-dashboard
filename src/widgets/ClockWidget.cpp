#include "ClockWidget.h"
#include <Arduino.h>

ClockWidget::ClockWidget(const std::string& id) noexcept
    : Widget(id)
{
}

void ClockWidget::update(const DashboardState& state) noexcept
{
    last_ = state;
}

void ClockWidget::render() const noexcept
{
    // Rendering delegated to Display; keep this minimal for compilation
}

