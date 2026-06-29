#pragma once

#include <string>
#include "geometry/Rectangle.h"

class Dashboard;

/**
 * @brief E-ink display driver with GxEPD2 support.
 * 
 * Implements partial updates for battery efficiency:
 * - Full refresh: Complete screen redraw (slow, high power)
 * - Partial refresh: Only changed regions (fast, low power)
 */
class Display
{
public:
    Display() noexcept;
    ~Display() = default;

    /**
     * @brief Perform full screen refresh (initialization, deep black required)
     */
    void init() noexcept;

    /**
     * @brief Render all widgets (full refresh)
     */
    void render(const Dashboard& dashboard) const noexcept;

    /**
     * @brief Partial update - only redraw dirty widgets
     * 
     * More efficient for frequent updates:
     * - Only regions with isDirty() = true are refreshed
     * - Reduces power consumption and update time
     */
    void partialUpdate(const Dashboard& dashboard) noexcept;

    /**
     * @brief Force full refresh on next update
     */
    void forceFullRefresh() noexcept { needsFullRefresh_ = true; }

private:
    void renderWidget(const Dashboard& dashboard, const std::string& widgetId) const noexcept;
    bool needsFullRefresh_ = true;
};
