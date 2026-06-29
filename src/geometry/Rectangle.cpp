#include "geometry/Rectangle.h"

Rectangle::Rectangle(const Point& position, const Size& size) noexcept
    : position_(position)
    , size_(size)
{
}

const Point& Rectangle::getPosition() const noexcept
{
    return position_;
}

const Size& Rectangle::getSize() const noexcept
{
    return size_;
}
