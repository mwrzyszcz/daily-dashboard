#include "services/ApplicationDiagnostics.h"

#include "Logger.h"

namespace
{
bool hasWeatherPayload(const Weather& weather) noexcept
{
    return weather.temperature != 0.0f || weather.humidity != 0 || weather.pressure != 0 || !weather.description.empty();
}
}

ApplicationDiagnostics::ApplicationDiagnostics(const Dashboard& dashboard,
                                               const IClockService& clockService,
                                               const Scheduler& scheduler) noexcept
    : dashboard_(dashboard)
    , clockService_(clockService)
    , scheduler_(scheduler)
{
}

void ApplicationDiagnostics::run() const noexcept
{
    Logger::info("Diagnostics", "Starting application diagnostics");

    int passedChecks = 0;
    int failedChecks = 0;

    const auto& state = dashboard_.getState();
    const bool timeSynchronized = clockService_.isTimeValid();
    logResult("Clock synchronization", timeSynchronized, timeSynchronized ? "NTP time valid" : "NTP sync not confirmed");
    timeSynchronized ? ++passedChecks : ++failedChecks;

    const bool schedulerRegistered = scheduler_.taskCount() >= 3;
    logResult("Scheduler registration", schedulerRegistered, schedulerRegistered ? "Periodic tasks registered" : "Expected at least 3 tasks");
    schedulerRegistered ? ++passedChecks : ++failedChecks;

    const bool schedulerRunning = scheduler_.isRunning();
    logResult("Scheduler runtime", schedulerRunning, schedulerRunning ? "Scheduler active" : "Scheduler not running");
    schedulerRunning ? ++passedChecks : ++failedChecks;

    const bool dateReady = hasValidDate(state.date);
    logResult("Dashboard date", dateReady, dateReady ? "Date propagated to dashboard state" : "Dashboard state has invalid date");
    dateReady ? ++passedChecks : ++failedChecks;

    const bool weatherReady = hasWeatherPayload(state.weather);
    logResult("Weather integration", weatherReady, weatherReady ? "Weather payload available" : "Weather data empty or defaulted");
    weatherReady ? ++passedChecks : ++failedChecks;

    const bool forecastReady = !state.forecast.empty();
    logResult("Forecast integration", forecastReady, forecastReady ? "Forecast entries loaded" : "Forecast data empty");
    forecastReady ? ++passedChecks : ++failedChecks;

    const bool calendarReady = dateReady && state.calendarMonth.year == state.date.year && state.calendarMonth.month == state.date.month && !state.calendarMonth.weeks.empty();
    logResult("Calendar integration", calendarReady, calendarReady ? "Calendar month derived from current date" : "Calendar month not built from dashboard date");
    calendarReady ? ++passedChecks : ++failedChecks;

    const bool namedayReady = !state.namedays.empty();
    logResult("Nameday integration", namedayReady, namedayReady ? "Nameday entries available" : "Nameday list empty");
    namedayReady ? ++passedChecks : ++failedChecks;

    const bool holidayConsistent = state.holiday.isHoliday == !state.holiday.name.empty();
    logResult("Holiday integration", holidayConsistent, holidayConsistent ? "Holiday state consistent" : "Holiday flag and name diverged");
    holidayConsistent ? ++passedChecks : ++failedChecks;

    char summary[96];
    std::snprintf(summary, sizeof(summary), "Diagnostics finished: %d passed, %d failed", passedChecks, failedChecks);
    Logger::info("Diagnostics", summary);
}

void ApplicationDiagnostics::logResult(const char* checkName, bool passed, const char* details) const noexcept
{
    char message[160];
    std::snprintf(message, sizeof(message), "%s: %s (%s)", checkName, passed ? "PASS" : "FAIL", details);
    if (passed) {
        Logger::info("Diagnostics", message);
        return;
    }

    Logger::warn("Diagnostics", message);
}

bool ApplicationDiagnostics::hasValidDate(const Date& date) const noexcept
{
    return date.year >= 2000 && date.month >= 1 && date.month <= 12 && date.day >= 1 && date.day <= 31;
}