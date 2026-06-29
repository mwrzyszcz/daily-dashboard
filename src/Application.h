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

/**
 * @brief Główna klasa aplikacji.
 * 
 * Uses FreeRTOS scheduler for battery-efficient periodic updates:
 * - Clock: every 1 second
 * - Weather: every 30 minutes
 * - Nameday: every 24 hours
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
                NamedayService& namedayService) noexcept;
    ~Application() = default;

    void setup() noexcept;
    void loop() noexcept;  // Minimal, kept for Arduino framework compatibility

private:
    void handleDeveloperCommands() noexcept;
    void runDiagnostics() noexcept;
    void initializeScheduler() noexcept;
    void refreshDashboardState() noexcept;
    void updateClock() noexcept;
    void refreshWeather() noexcept;
    void refreshNameday() noexcept;

    Configuration& configuration_;
    Dashboard& dashboard_;
    Display& display_;
    IClockService& clockService_;
    IWeatherService& weatherService_;
    CalendarService& calendarService_;
    HolidayService& holidayService_;
    NamedayService& namedayService_;
    
    Scheduler scheduler_;
};
