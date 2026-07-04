#include "Dashboard.h"

Dashboard::Dashboard(Layout& layout) noexcept
    : layout_(layout)
{
}

void Dashboard::setState(DashboardState state) noexcept
{
    state_ = std::move(state);
    needsRefresh_ = true;
    for (auto* widget : widgets_) {
        if (widget) {
            widget->update(state_);
        }
    }
}

const DashboardState& Dashboard::getState() const noexcept
{
    return state_;
}

void Dashboard::update() noexcept
{
    layout_.measure();
    layout_.layout();
    needsRefresh_ = true;
}

void Dashboard::render() const noexcept
{
    if (!needsRefresh_) {
        return;
    }

    layout_.render();
}

void Dashboard::invalidate() noexcept
{
    needsRefresh_ = true;
}

void Dashboard::addWidget(Widget& widget) noexcept
{
    widgets_.push_back(&widget);
    layout_.addChild(widget);
    needsRefresh_ = true;
}

Widget* Dashboard::getWidget(const std::string& id) noexcept
{
    for (auto* widget : widgets_) {
        if (widget && widget->getId() == id) {
            return widget;
        }
    }
    return nullptr;
}

const Widget* Dashboard::getWidget(const std::string& id) const noexcept
{
    for (auto* widget : widgets_) {
        if (widget && widget->getId() == id) {
            return widget;
        }
    }
    return nullptr;
}

const std::vector<Widget*>& Dashboard::getWidgets() const noexcept
{
    return widgets_;
}
