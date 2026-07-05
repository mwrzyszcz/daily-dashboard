#include "CurrencyWidget.h"

CurrencyWidget::CurrencyWidget(const std::string& id) noexcept
    : Widget(id)
{
}

void CurrencyWidget::update(const DashboardState& state) noexcept
{
    last_ = state;
}

void CurrencyWidget::render() const noexcept
{
}
