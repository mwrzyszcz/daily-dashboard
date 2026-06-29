#pragma once

#include <cstdint>

struct Size
{
    Size() noexcept = default;
    Size(int16_t widthValue, int16_t heightValue) noexcept
        : width(widthValue)
        , height(heightValue)
    {
    }

    int16_t width = 0;
    int16_t height = 0;
};
