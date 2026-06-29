#pragma once

#include <string>
#include "geometry/Point.h"
#include "geometry/Size.h"
#include "geometry/Rectangle.h"
#include "models/DashboardState.h"

/**
 * @brief Podstawowy widget pulpitu.
 */
class Widget
{
public:
    Widget(const std::string& id) noexcept;
    virtual ~Widget() = default;

    const std::string& getId() const noexcept;
    const Point& getPosition() const noexcept;
    const Size& getSize() const noexcept;
    const Rectangle& getBounds() const noexcept;

    bool isVisible() const noexcept;
    void setPreferredSize(const Size& size) noexcept;
    const Size& getPreferredSize() const noexcept;

    virtual void measure() noexcept;
    virtual void layout(const Rectangle& bounds) noexcept;
    virtual void render() const noexcept;
    virtual void update(const DashboardState& state) noexcept;
    virtual void invalidate() noexcept;
    
    bool isDirty() const noexcept { return isDirty_; }
    void markClean() noexcept { isDirty_ = false; }

protected:
    std::string id_;
    Point position_;
    Size size_;
    Rectangle bounds_;
    bool visible_ = true;
    Size preferredSize_;
    bool isDirty_ = true;  // Track if widget needs redraw
};
