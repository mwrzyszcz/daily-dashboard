#pragma once

#include "Point.h"
#include "Size.h"

class Rectangle
{
public:
    Rectangle() = default;
    Rectangle(const Point& position, const Size& size) noexcept;

    const Point& getPosition() const noexcept;
    const Size& getSize() const noexcept;

private:
    Point position_;
    Size size_;
};
