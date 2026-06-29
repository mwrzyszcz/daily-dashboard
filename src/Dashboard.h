#pragma once

#include <vector>
#include "layout/Layout.h"
#include "widgets/Widget.h"
#include "models/DashboardState.h"

/**
 * @brief Zarządza kolekcją widgetów oraz stanem pulpitu.
 */
class Dashboard
{
public:
    explicit Dashboard(Layout& layout) noexcept;
    ~Dashboard() = default;

    void setState(DashboardState state) noexcept;
    const DashboardState& getState() const noexcept;

    void update() noexcept;
    void render() const noexcept;
    void invalidate() noexcept;

    void addWidget(Widget& widget) noexcept;
    Widget* getWidget(const std::string& id) noexcept;
    const Widget* getWidget(const std::string& id) const noexcept;

private:
    Layout& layout_;
    DashboardState state_;
    std::vector<Widget*> widgets_;
    bool needsRefresh_ = true;
};
