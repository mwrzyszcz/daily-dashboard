#include "Application.h"

#include <cstring>
#include <utility>

#include <Arduino.h>
#include <WiFi.h>
#include "Logger.h"
#include "models/DashboardState.h"
#include "services/ApplicationDiagnostics.h"

namespace
{
constexpr size_t DIAGNOSTIC_COMMAND_BUFFER_SIZE = 32;

bool isValidDate(const Date& date) noexcept
{
    return date.year >= 2000 && date.month >= 1 && date.month <= 12 && date.day >= 1 && date.day <= 31;
}

bool isSameDate(const Date& left, const Date& right) noexcept
{
    return left.year == right.year && left.month == right.month && left.day == right.day;
}
}

Application::Application(Configuration& configuration,
                         Dashboard& dashboard,
                         Display& display,
                         IClockService& clockService,
                         IWeatherService& weatherService,
                         CalendarService& calendarService,
                         HolidayService& holidayService,
                         NamedayService& namedayService) noexcept
    : configuration_(configuration)
    , dashboard_(dashboard)
    , display_(display)
    , clockService_(clockService)
    , weatherService_(weatherService)
    , calendarService_(calendarService)
    , holidayService_(holidayService)
    , namedayService_(namedayService)
{
}

void Application::setup() noexcept
{
    Logger::info("Application", "Initializing services");

    display_.init();
    connectWiFi();
    clockService_.begin();

    refreshDashboardState();
    dashboard_.update();
    display_.render(dashboard_);

    initializeScheduler();
    scheduler_.start();

    Logger::info("Application", "Developer commands: diag, help");
}

void Application::loop() noexcept
{
    handleDeveloperCommands();
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void Application::connectWiFi() noexcept
{
    const char* ssid     = configuration_.getWifiSSID().c_str();
    const char* password = configuration_.getWifiPassword().c_str();

    Logger::info("WiFi", "Connecting...");
    WiFi.begin(ssid, password);

    constexpr int MAX_ATTEMPTS = 20;
    for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
        if (WiFi.status() == WL_CONNECTED) {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "Connected, IP: %s", WiFi.localIP().toString().c_str());
            Logger::info("WiFi", buf);
            return;
        }
        delay(500);
    }

    Logger::warn("WiFi", "Connection failed — services requiring network will not work");
}

void Application::handleDeveloperCommands() noexcept
{
    static char commandBuffer[DIAGNOSTIC_COMMAND_BUFFER_SIZE] = {};
    static size_t commandLength = 0;

    while (Serial.available() > 0) {
        const char raw = static_cast<char>(Serial.read());
        if (raw == '\r') {
            continue;
        }

        if (raw == '\n') {
            if (commandLength == 0) {
                continue;
            }

            commandBuffer[commandLength] = '\0';
            if (std::strcmp(commandBuffer, "diag") == 0) {
                runDiagnostics();
            } else if (std::strcmp(commandBuffer, "help") == 0) {
                Logger::info("Application", "Available commands: diag, help");
            } else {
                Logger::warn("Application", "Unknown developer command. Use 'help'");
            }

            commandLength = 0;
            commandBuffer[0] = '\0';
            continue;
        }

        if (commandLength + 1 >= DIAGNOSTIC_COMMAND_BUFFER_SIZE) {
            commandLength = 0;
            commandBuffer[0] = '\0';
            Logger::warn("Application", "Developer command too long");
            continue;
        }

        commandBuffer[commandLength++] = raw;
    }
}

void Application::runDiagnostics() noexcept
{
    Logger::info("Application", "Running developer-requested diagnostics");
    refreshDashboardState();
    dashboard_.update();

    ApplicationDiagnostics diagnostics(dashboard_, clockService_, scheduler_);
    diagnostics.run();
}

void Application::initializeScheduler() noexcept
{
    scheduler_.everyMinutes(configuration_.getClockRefreshMinutes(), [this]() {
        updateClock();
    });
    scheduler_.everyMinutes(configuration_.getWeatherRefreshMinutes(), [this]() {
        refreshWeather();
    });
    // Imieniny, kalendarz i święta odświeżane są chwilę po północy przy zmianie daty (patrz updateClock()),
    // dlatego nie ma tu osobnego zadania czasowego.
}

void Application::refreshDashboardState() noexcept
{
    DashboardState state = dashboard_.getState();
    const Date currentDate = clockService_.getDate();
    const Time currentTime = clockService_.getTime();

    state.date = currentDate;
    state.time = currentTime;
    state.weather = weatherService_.getCurrentWeather();
    state.forecast = weatherService_.getForecast();
    state.weatherUpdatedDate = currentDate;
    state.weatherUpdatedTime = currentTime;

    if (isValidDate(currentDate)) {
        state.calendarMonth = calendarService_.getCurrentMonth(currentDate);
        state.holiday = holidayService_.getHolidayInfo(currentDate);
        state.namedays = namedayService_.getNamedays(currentDate);
    } else {
        state.calendarMonth = {};
        state.holiday = {};
        state.namedays.clear();
    }

    dashboard_.setState(std::move(state));
}

void Application::updateClock() noexcept
{
    DashboardState state = dashboard_.getState();
    const Date previousDate = state.date;
    const Date currentDate = clockService_.getDate();
    const Time currentTime = clockService_.getTime();

    state.date = currentDate;
    state.time = currentTime;
    dashboard_.setState(std::move(state));

    const bool dateChanged = isValidDate(currentDate) && !isSameDate(previousDate, currentDate);

    if (currentTime.minutes != lastDisplayedMinute_) {
        lastDisplayedMinute_ = currentTime.minutes;
        if (Widget* w = dashboard_.getWidget("clock_widget")) w->invalidate();
        if (dateChanged) {
            // Zmiana daty = chwilę po północy: odśwież imieniny, kalendarz i święta.
            refreshNameday();
        }
        display_.partialUpdate(dashboard_);
    }
}

void Application::refreshWeather() noexcept
{
    DashboardState state = dashboard_.getState();
    state.weather = weatherService_.getCurrentWeather();
    state.forecast = weatherService_.getForecast();
    state.weatherUpdatedDate = clockService_.getDate();
    state.weatherUpdatedTime = clockService_.getTime();

    dashboard_.setState(std::move(state));
    if (Widget* w = dashboard_.getWidget("weather_widget"))  w->invalidate();
    if (Widget* w = dashboard_.getWidget("forecast_widget")) w->invalidate();
    if (Widget* w = dashboard_.getWidget("status_bar_widget")) w->invalidate();
    display_.partialUpdate(dashboard_);
}

// Odświeża imieniny, kalendarz i święta dla bieżącej daty. Wołane chwilę po północy z updateClock().
// Nie wykonuje samo partialUpdate() — robi to wywołujący po zebraniu wszystkich brudnych widgetów.
void Application::refreshNameday() noexcept
{
    DashboardState state = dashboard_.getState();
    const Date currentDate = clockService_.getDate();

    if (!isValidDate(currentDate)) {
        Logger::warn("Application", "Skipping nameday refresh due to invalid date");
        return;
    }

    state.date = currentDate;
    state.calendarMonth = calendarService_.getCurrentMonth(currentDate);
    state.holiday = holidayService_.getHolidayInfo(currentDate);
    state.namedays = namedayService_.getNamedays(currentDate);

    dashboard_.setState(std::move(state));
    if (Widget* w = dashboard_.getWidget("nameday_widget"))  w->invalidate();
    if (Widget* w = dashboard_.getWidget("holiday_widget"))  w->invalidate();
    if (Widget* w = dashboard_.getWidget("calendar_widget")) w->invalidate();
}