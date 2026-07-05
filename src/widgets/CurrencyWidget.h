#pragma once

#include "Widget.h"

class CurrencyWidget : public Widget
{
public:
    CurrencyWidget(const std::string& id) noexcept;
    ~CurrencyWidget() override = default;

    void update(const DashboardState& state) noexcept override;
    void render() const noexcept override;

private:
    DashboardState last_{};
};
