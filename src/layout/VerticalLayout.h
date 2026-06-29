#pragma once

#include "layout/Layout.h"

class VerticalLayout : public Layout
{
public:
    explicit VerticalLayout(const std::string& id) noexcept;
    ~VerticalLayout() override = default;

    void measure() noexcept override;
    void layout() noexcept override;
};
