#pragma once

#include "widgets/Widget.h"
#include "models/DashboardState.h"

class StatusBarWidget : public Widget
{
public:
    explicit StatusBarWidget(const std::string& id) noexcept;
    ~StatusBarWidget() override = default;

    void update(const DashboardState& state) noexcept override;
    void render() const noexcept override;

private:
    std::string statusText_;
};
