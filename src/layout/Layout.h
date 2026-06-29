#pragma once

#include <vector>
#include "widgets/Widget.h"
#include "geometry/Rectangle.h"

/**
 * @brief Abstrakcyjny layout układający widgety.
 */
class Layout
{
public:
    explicit Layout(const std::string& id) noexcept;
    virtual ~Layout() = default;

    const std::string& getId() const noexcept;
    void addChild(Widget& widget) noexcept;
    const std::vector<Widget*>& getChildren() const noexcept;

    virtual void measure() noexcept;
    virtual void layout() noexcept;
    virtual void render() const noexcept;

protected:
    std::string id_;
    std::vector<Widget*> children_;
    Rectangle bounds_;
};
