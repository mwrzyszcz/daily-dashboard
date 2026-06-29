#pragma once

#include "Widget.h"

class NamedayWidget : public Widget
{
public:
    NamedayWidget(const std::string& id) noexcept;
    ~NamedayWidget() override = default;

    void update(const DashboardState& state) noexcept override;
    void render() const noexcept override;

private:
    DashboardState last_{};
};

