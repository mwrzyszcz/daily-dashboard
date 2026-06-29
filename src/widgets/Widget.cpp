#include "widgets/Widget.h"

Widget::Widget(const std::string& id) noexcept
    : id_(id)
{
}

const std::string& Widget::getId() const noexcept
{
    return id_;
}

const Point& Widget::getPosition() const noexcept
{
    return position_;
}

const Size& Widget::getSize() const noexcept
{
    return size_;
}

const Rectangle& Widget::getBounds() const noexcept
{
    return bounds_;
}

bool Widget::isVisible() const noexcept
{
    return visible_;
}

void Widget::setPreferredSize(const Size& size) noexcept
{
    preferredSize_ = size;
}

const Size& Widget::getPreferredSize() const noexcept
{
    return preferredSize_;
}

void Widget::measure() noexcept
{
    size_ = preferredSize_;
}

void Widget::layout(const Rectangle& bounds) noexcept
{
    bounds_ = bounds;
    position_ = bounds.getPosition();
    size_ = bounds.getSize();
}

void Widget::render() const noexcept
{
    // Podstawowy widget nie renderuje.
}

void Widget::update(const DashboardState& state) noexcept
{
    (void)state;
}

void Widget::invalidate() noexcept
{
    isDirty_ = true;  // Mark widget as needing redraw
}
