#pragma once

#include <cstdint>

struct Point
{
    Point() noexcept = default;
    Point(int16_t xValue, int16_t yValue) noexcept
        : x(xValue)
        , y(yValue)
    {
    }

    int16_t x = 0;
    int16_t y = 0;
};
