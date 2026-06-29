#include "layout/VerticalLayout.h"

VerticalLayout::VerticalLayout(const std::string& id) noexcept
    : Layout(id)
{
}

void VerticalLayout::measure() noexcept
{
    Layout::measure();
}

void VerticalLayout::layout() noexcept
{
    int y = 0;
    for (Widget* child : children_) {
        if (child && child->isVisible()) {
            const Size childSize = child->getPreferredSize();
            Rectangle childBounds(Point{0, y}, childSize);
            child->layout(childBounds);
            y += childSize.height;
        }
    }
}
