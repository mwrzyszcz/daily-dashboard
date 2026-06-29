#pragma once

#include "models/Date.h"
#include "models/Holiday.h"
#include <optional>
#include <string>

class HolidayService
{
public:
    HolidayService() = default;
    ~HolidayService() = default;

    std::string getHoliday(const Date& date) const;
    bool isHoliday(const Date& date) const;
    
    // Returns Holiday struct with isPublicHoliday info
    Holiday getHolidayInfo(const Date& date) const;

private:
    static Date easterSunday(uint16_t year);
    static Date addDays(const Date& d, int offset);
    static bool isPublicHolidayDate(const Date& date);
};

