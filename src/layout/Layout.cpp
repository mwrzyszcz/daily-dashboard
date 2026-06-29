#include "layout/Layout.h"

Layout::Layout(const std::string& id) noexcept
    : id_(id)
{
}

const std::string& Layout::getId() const noexcept
{
    return id_;
}

void Layout::addChild(Widget& widget) noexcept
{
    children_.push_back(&widget);
}

const std::vector<Widget*>& Layout::getChildren() const noexcept
{
    return children_;
}

void Layout::measure() noexcept
{
    for (Widget* child : children_) {
        if (child) {
            child->measure();
        }
    }
}

void Layout::layout() noexcept
{
    const Rectangle layoutBounds(Point{0, 0}, Size{800, 480});
    for (Widget* child : children_) {
        if (child) {
            child->layout(layoutBounds);
        }
    }
}

void Layout::render() const noexcept
{
    for (const Widget* child : children_) {
        if (child && child->isVisible()) {
            child->render();
        }
    }
}
