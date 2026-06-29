#include "widgets/StatusBarWidget.h"
#include "Logger.h"
#include <Arduino.h>

StatusBarWidget::StatusBarWidget(const std::string& id) noexcept
    : Widget(id)
{
    setPreferredSize(Size{800, 40});
}

void StatusBarWidget::update(const DashboardState& state) noexcept
{
    statusText_.clear();
    statusText_ = "WiFi: online | Ostatnia synchronizacja: ";
    statusText_ += state.time.toString();
}

void StatusBarWidget::render() const noexcept
{
    std::string msg = "[StatusBar] " + statusText_;
    Logger::info("StatusBarWidget", msg.c_str());
}
