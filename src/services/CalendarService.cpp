#include "services/CalendarService.h"
#include <ctime>

static bool isLeapYear(int y)
{
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

uint8_t CalendarService::daysInMonth(uint16_t year, uint8_t month) noexcept
{
    static const uint8_t mdays[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
    if (month == 2 && isLeapYear(year)) return 29;
    return mdays[month];
}

uint8_t CalendarService::firstWeekday(uint16_t year, uint8_t month) noexcept
{
    std::tm t{};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = 1;
    std::mktime(&t);
    // tm_wday: 0 = Sunday .. 6 = Saturday; convert to 1 = Monday .. 7 = Sunday
    return (t.tm_wday == 0) ? 7 : static_cast<uint8_t>(t.tm_wday);
}

uint8_t CalendarService::weekNumber(const Date& date) noexcept
{
    std::tm t{};
    t.tm_year = date.year - 1900;
    t.tm_mon = date.month - 1;
    t.tm_mday = date.day;
    std::mktime(&t);
    int doy = t.tm_yday + 1;
    int wday = (t.tm_wday == 0) ? 7 : t.tm_wday; // 1..7
    int week = (doy - wday + 10) / 7;
    return static_cast<uint8_t>(week);
}

bool CalendarService::isWeekend(uint8_t weekday) noexcept
{
    return weekday == 6 || weekday == 7;
}

bool CalendarService::isToday(const Date& a, const Date& b) noexcept
{
    return a.year == b.year && a.month == b.month && a.day == b.day;
}

CalendarMonth CalendarService::getCurrentMonth(const Date& date) const noexcept
{
    CalendarMonth cm;
    cm.year = date.year;
    cm.month = date.month;

    const uint8_t firstWd = firstWeekday(cm.year, cm.month); // 1=Mon..7=Sun
    const uint8_t dim = daysInMonth(cm.year, cm.month);

    // create weeks, starting Monday
    uint8_t day = 1;
    int weekDayIndex = firstWd == 7 ? 6 : firstWd - 1; // 0=Mon..6=Sun

    while (day <= dim) {
        CalendarWeek week;
        for (int i = 0; i < 7; ++i) {
            CalendarDay cd;
            if (cm.weeks.empty() && i < weekDayIndex) {
                cd.inMonth = false;
            } else if (day > dim) {
                cd.inMonth = false;
            } else {
                cd.inMonth = true;
                cd.day = day;
                cd.isWeekend = (i >= 5);
                ++day;
            }
            week.days[i] = cd;
        }
        cm.weeks.push_back(week);
    }

    return cm;
}

