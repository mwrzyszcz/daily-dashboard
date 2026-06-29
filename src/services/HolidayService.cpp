#include "HolidayService.h"
#include "Logger.h"
#include "data/Holidays.h"
#include <Arduino.h>
#include <ctime>
#include <unordered_map>

// Meeus/Jones/Butcher algorithm for Easter Sunday (Gregorian)
Date HolidayService::easterSunday(uint16_t Y)
{
    int a = Y % 19;
    int b = Y / 100;
    int c = Y % 100;
    int d = b / 4;
    int e = b % 4;
    int f = (b + 8) / 25;
    int g = (b - f + 1) / 3;
    int h = (19 * a + b - d - g + 15) % 30;
    int i = c / 4;
    int k = c % 4;
    int l = (32 + 2 * e + 2 * i - h - k) % 7;
    int m = (a + 11 * h + 22 * l) / 451;
    int month = (h + l - 7 * m + 114) / 31;
    int day = ((h + l - 7 * m + 114) % 31) + 1;
    return Date(Y, static_cast<uint8_t>(month), static_cast<uint8_t>(day));
}

Date HolidayService::addDays(const Date& d, int offset)
{
    std::tm t{};
    t.tm_year = d.year - 1900;
    t.tm_mon = d.month - 1;
    t.tm_mday = d.day + offset;
    std::mktime(&t);
    Date r;
    r.year = static_cast<uint16_t>(t.tm_year + 1900);
    r.month = static_cast<uint8_t>(t.tm_mon + 1);
    r.day = static_cast<uint8_t>(t.tm_mday);
    r.weekday = (t.tm_wday == 0) ? 7 : static_cast<uint8_t>(t.tm_wday);
    return r;
}

bool HolidayService::isPublicHolidayDate(const Date& date)
{
    uint16_t key = static_cast<uint16_t>(date.month) * 100 + date.day;
    
    // Public holidays in Poland (urzędowo wolne dni pracy)
    static const std::unordered_map<uint16_t, bool> PUBLIC_HOLIDAYS = {
        {101, true},    // New Year
        {501, true},    // Labor Day
        {511, true},    // Constitution Day
        {811, true},    // Assumption
        {1111, true},   // Independence Day
        {1225, true},   // Christmas Day
        {1226, true},   // Second Day of Christmas
    };
    
    auto it = PUBLIC_HOLIDAYS.find(key);
    if (it != PUBLIC_HOLIDAYS.end()) {
        return it->second;
    }
    
    // Movable public holidays
    Date easter = easterSunday(date.year);
    Date easterMonday = addDays(easter, 1);
    Date pentecost = addDays(easter, 49);

    if (date.year == easter.year && date.month == easter.month && date.day == easter.day) return true;
    if (date.year == easterMonday.year && date.month == easterMonday.month && date.day == easterMonday.day) return true;
    if (date.year == pentecost.year && date.month == pentecost.month && date.day == pentecost.day) return true;
    
    return false;
}

std::string HolidayService::getHoliday(const Date& date) const
{
    uint16_t key = static_cast<uint16_t>(date.month) * 100 + date.day;
    auto it = HOLIDAYS_FIXED.find(key);
    if (it != HOLIDAYS_FIXED.end()) {
        return it->second;
    }

    // Movable feasts
    Date easter = easterSunday(date.year);
    Date easterMonday = addDays(easter, 1);
    Date pentecost = addDays(easter, 49);
    Date corpusChristi = addDays(easter, 60);
    Logger::info("Holiday", "Calculated movable holidays");

    if (date.year == easter.year && date.month == easter.month && date.day == easter.day) return std::string("Wielkanoc");
    if (date.year == easterMonday.year && date.month == easterMonday.month && date.day == easterMonday.day) return std::string("Poniedziałek Wielkanocny");
    if (date.year == pentecost.year && date.month == pentecost.month && date.day == pentecost.day) return std::string("Zielone Świątki");
    if (date.year == corpusChristi.year && date.month == corpusChristi.month && date.day == corpusChristi.day) return std::string("Boże Ciało");

    return std::string();
}

bool HolidayService::isHoliday(const Date& date) const
{
    return !getHoliday(date).empty();
}

Holiday HolidayService::getHolidayInfo(const Date& date) const
{
    Holiday holiday{};
    std::string holidayName = getHoliday(date);
    
    if (!holidayName.empty()) {
        holiday.isHoliday = true;
        holiday.name = holidayName;
        holiday.isPublicHoliday = isPublicHolidayDate(date);
    }
    
    return holiday;
}

