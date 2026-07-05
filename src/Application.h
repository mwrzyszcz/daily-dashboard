#pragma once

#include "Configuration.h"
#include "Dashboard.h"
#include "Display.h"
#include "scheduler/Scheduler.h"
#include "interfaces/IClockService.h"
#include "interfaces/IWeatherService.h"
#include "services/ClockService.h"
#include "services/WeatherService.h"
#include "services/CalendarService.h"
#include "services/HolidayService.h"
#include "services/NamedayService.h"
#include "services/CurrencyService.h"

/**
 * @brief Główna klasa aplikacji.
 * 
 * Uses FreeRTOS scheduler for battery-efficient periodic updates.
 * Refresh intervals per widget are configurable (see Configuration):
 * - Clock: every getClockRefreshMinutes() minutes (default 1) — only the clock region is redrawn
 * - Weather + forecast: every getWeatherRefreshMinutes() minutes (default 60)
 * - Nameday/calendar: shortly after midnight (triggered on date change in updateClock())
 */
class Application
{
public:
    Application(Configuration& configuration,
                Dashboard& dashboard,
                Display& display,
                IClockService& clockService,
                IWeatherService& weatherService,
                CalendarService& calendarService,
                HolidayService& holidayService,
                NamedayService& namedayService,
                CurrencyService& currencyService) noexcept;
    ~Application() = default;

    void setup() noexcept;
    void loop() noexcept;  // Minimal, kept for Arduino framework compatibility

private:
    void handleDeveloperCommands() noexcept;
    void runDiagnostics() noexcept;
    void connectWiFi() noexcept;
    void initializeScheduler() noexcept;
    void refreshDashboardState() noexcept;
    void updateClock() noexcept;
    void refreshWeather() noexcept;
    void refreshNameday() noexcept;
    void refreshCurrency() noexcept;

    Configuration& configuration_;
    Dashboard& dashboard_;
    Display& display_;
    IClockService& clockService_;
    IWeatherService& weatherService_;
    CalendarService& calendarService_;
    HolidayService& holidayService_;
    NamedayService& namedayService_;
    CurrencyService& currencyService_;
    
    Scheduler scheduler_;
    uint8_t lastDisplayedMinute_ = 255;  // 255 = not yet rendered
};
