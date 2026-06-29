#include "services/ClockService.h"
#include "Logger.h"
#include <Arduino.h>
#include <ctime>
#include <cstdlib>
#include <sys/time.h>

static constexpr const char* TZ_POLAND = "CET-1CEST,M3.5.0/2,M10.5.0/3";

void ClockService::begin()
{
    Logger::info("NTP", "Configuring time synchronization");
    configTzTime(TZ_POLAND, "pool.ntp.org", "time.google.com");
    synchronize();
}

void ClockService::synchronize()
{
    struct tm timeinfo;
    for (int i = 0; i < 10; ++i) {
        if (getLocalTime(&timeinfo)) {
            timeValid_.store(true);
            Logger::info("NTP", "Time synchronized");
            return;
        }
        delay(1000);
    }
    timeValid_.store(false);
    Logger::warn("NTP", "Synchronization failed");
}

bool ClockService::isTimeValid() const
{
    return timeValid_.load();
}

static void fillDateFromTm(const tm& info, Date& out)
{
    out.year = static_cast<uint16_t>(info.tm_year + 1900);
    out.month = static_cast<uint8_t>(info.tm_mon + 1);
    out.day = static_cast<uint8_t>(info.tm_mday);
    out.weekday = (info.tm_wday == 0) ? 7 : static_cast<uint8_t>(info.tm_wday);
    out.dayOfYear = static_cast<uint16_t>(info.tm_yday + 1);
    char buf[8] = {};
    tm tmp = info;
    strftime(buf, sizeof(buf), "%V", &tmp);
    out.weekNumber = static_cast<uint8_t>(std::atoi(buf));
}

Date ClockService::getDate() const
{
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        Date d;
        fillDateFromTm(timeinfo, d);
        return d;
    }
    return Date{0, 0, 0};
}

Time ClockService::getTime() const
{
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        return Time{static_cast<uint8_t>(timeinfo.tm_hour), static_cast<uint8_t>(timeinfo.tm_min)};
    }
    return Time{0, 0};
}

std::string ClockService::getFormattedDate() const
{
    auto d = getDate();
    static const char* months[] = {"", "stycznia", "lutego", "marca", "kwietnia", "maja", "czerwca", "lipca", "sierpnia", "wrzesnia", "pazdziernika", "listopada", "grudnia"};
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%u %s %u", d.day, months[d.month], d.year);
    return std::string(buf);
}

std::string ClockService::getFormattedTime() const
{
    auto t = getTime();
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%02u:%02u", t.hours, t.minutes);
    return std::string(buf);
}
