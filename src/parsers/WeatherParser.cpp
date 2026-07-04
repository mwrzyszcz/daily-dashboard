#include "parsers/WeatherParser.h"
#include "Logger.h"
#include "models/WeatherIcon.h"
#include <ArduinoJson.h>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <ctime>

namespace
{
struct ForecastAggregate
{
    Date date{};
    float minTemp = 0.0f;
    float maxTemp = 0.0f;
    WeatherIcon icon = WeatherIcon::Unknown;
    int bestHourDistance = 100;
    bool initialized = false;
};

bool parseDateToken(const char* dateText, Date& date) noexcept
{
    if (!dateText) {
        return false;
    }

    int year = 0;
    int month = 0;
    int day = 0;
    if (std::sscanf(dateText, "%d-%d-%d", &year, &month, &day) != 3) {
        return false;
    }

    tm timeInfo = {};
    timeInfo.tm_year = year - 1900;
    timeInfo.tm_mon = month - 1;
    timeInfo.tm_mday = day;
    if (mktime(&timeInfo) == static_cast<time_t>(-1)) {
        return false;
    }

    date = Date{static_cast<uint16_t>(year), static_cast<uint8_t>(month), static_cast<uint8_t>(day)};
    date.weekday = timeInfo.tm_wday == 0 ? 7 : static_cast<uint8_t>(timeInfo.tm_wday);
    date.dayOfYear = static_cast<uint16_t>(timeInfo.tm_yday + 1);
    char weekBuffer[8] = {};
    strftime(weekBuffer, sizeof(weekBuffer), "%V", &timeInfo);
    date.weekNumber = static_cast<uint8_t>(std::atoi(weekBuffer));
    return true;
}

bool parseUnixTimestamp(const uint32_t unixTimestamp, Date& date) noexcept
{
    if (unixTimestamp == 0) {
        return false;
    }

    const time_t raw = static_cast<time_t>(unixTimestamp);
    tm timeInfo = {};
    if (localtime_r(&raw, &timeInfo) == nullptr) {
        return false;
    }

    date = Date{static_cast<uint16_t>(timeInfo.tm_year + 1900),
                static_cast<uint8_t>(timeInfo.tm_mon + 1),
                static_cast<uint8_t>(timeInfo.tm_mday)};
    date.weekday = timeInfo.tm_wday == 0 ? 7 : static_cast<uint8_t>(timeInfo.tm_wday);
    date.dayOfYear = static_cast<uint16_t>(timeInfo.tm_yday + 1);
    char weekBuffer[8] = {};
    strftime(weekBuffer, sizeof(weekBuffer), "%V", &timeInfo);
    date.weekNumber = static_cast<uint8_t>(std::atoi(weekBuffer));
    return true;
}

bool getToday(Date& date) noexcept
{
    const time_t raw = time(nullptr);
    if (raw == static_cast<time_t>(-1)) {
        return false;
    }

    tm timeInfo = {};
    if (localtime_r(&raw, &timeInfo) == nullptr) {
        return false;
    }

    date = Date{static_cast<uint16_t>(timeInfo.tm_year + 1900),
                static_cast<uint8_t>(timeInfo.tm_mon + 1),
                static_cast<uint8_t>(timeInfo.tm_mday)};
    return true;
}

int hourDistanceToMidday(const char* dateTimeText) noexcept
{
    if (!dateTimeText) {
        return 100;
    }

    int hour = 0;
    if (std::sscanf(dateTimeText, "%*d-%*d-%*d %d:%*d:%*d", &hour) != 1) {
        return 100;
    }

    return std::abs(hour - 12);
}

int hourDistanceToMidday(const uint32_t unixTimestamp) noexcept
{
    if (unixTimestamp == 0) {
        return 100;
    }

    const time_t raw = static_cast<time_t>(unixTimestamp);
    tm timeInfo = {};
    if (localtime_r(&raw, &timeInfo) == nullptr) {
        return 100;
    }

    return std::abs(timeInfo.tm_hour - 12);
}

JsonDocument buildForecastFilter()
{
    JsonDocument filter;
    filter["list"][0]["dt"] = true;
    filter["list"][0]["dt_txt"] = true;
    filter["list"][0]["main"]["temp"] = true;
    filter["list"][0]["main"]["temp_min"] = true;
    filter["list"][0]["main"]["temp_max"] = true;
    filter["list"][0]["weather"][0]["icon"] = true;
    filter["city"]["sunrise"] = true;
    filter["city"]["sunset"] = true;
    return filter;
}
}

static WeatherIcon iconFromString(const std::string& icon)
{
    if (icon == "01d") return WeatherIcon::ClearDay;
    if (icon == "01n") return WeatherIcon::ClearNight;
    if (icon == "02d" || icon == "02n") return WeatherIcon::FewClouds;
    if (icon == "03d" || icon == "03n") return WeatherIcon::ScatteredClouds;
    if (icon == "04d" || icon == "04n") return WeatherIcon::BrokenClouds;
    if (icon == "09d" || icon == "09n") return WeatherIcon::ShowerRain;
    if (icon == "10d" || icon == "10n") return WeatherIcon::Rain;
    if (icon == "11d" || icon == "11n") return WeatherIcon::Thunderstorm;
    if (icon == "13d" || icon == "13n") return WeatherIcon::Snow;
    if (icon == "50d" || icon == "50n") return WeatherIcon::Mist;
    return WeatherIcon::Unknown;
}

Weather WeatherParser::parseCurrentWeather(const std::string& json) const
{
    (void)json;
    Weather weather;
    weather.temperature = 18.3f;
    weather.feelsLike = 17.8f;
    weather.tempMin = 16.0f;
    weather.tempMax = 19.2f;
    weather.humidity = 62;
    weather.pressure = 1014;
    weather.sunrise = 1624935600;
    weather.sunset = 1624990800;
    weather.icon = iconFromString("02d");
    weather.description = "Lekko zachmurzone";
    return weather;
}

std::vector<Forecast> WeatherParser::parseForecast(const std::string& json) const
{
    std::vector<Forecast> forecast;
    if (json.empty()) {
        Logger::warn("WeatherParser", "Forecast payload is empty");
        return forecast;
    }

    JsonDocument filter = buildForecastFilter();
    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc,
                                                       json.c_str(),
                                                       DeserializationOption::Filter(filter));
    if (error) {
        Logger::error("WeatherParser", std::string("Forecast JSON parse failed: ") + error.c_str());
        return forecast;
    }

    JsonArray entries = doc["list"].as<JsonArray>();
    if (entries.isNull()) {
        Logger::warn("WeatherParser", "Forecast payload has no list array");
        return forecast;
    }

    const uint32_t citySunrise = doc["city"]["sunrise"] | 0U;
    const uint32_t citySunset = doc["city"]["sunset"] | 0U;

    std::vector<ForecastAggregate> aggregates;
    aggregates.reserve(5);

    Date today;
    const bool hasToday = getToday(today);

    for (JsonObject entry : entries) {
        const char* dateTimeText = entry["dt_txt"] | nullptr;
        const uint32_t unixTimestamp = entry["dt"] | 0U;
        Date date;
        if (!parseDateToken(dateTimeText, date) && !parseUnixTimestamp(unixTimestamp, date)) {
            continue;
        }

        if (hasToday && date.year == today.year && date.month == today.month && date.day == today.day) {
            continue;
        }

        const float minTemp = entry["main"]["temp_min"] | entry["main"]["temp"] | 0.0f;
        const float maxTemp = entry["main"]["temp_max"] | entry["main"]["temp"] | 0.0f;
        const char* iconText = entry["weather"][0]["icon"] | "";
        const WeatherIcon icon = iconFromString(iconText);
        int middayDistance = hourDistanceToMidday(dateTimeText);
        if (middayDistance == 100) {
            middayDistance = hourDistanceToMidday(unixTimestamp);
        }

        ForecastAggregate* aggregate = nullptr;
        for (auto& candidate : aggregates) {
            if (candidate.date.year == date.year && candidate.date.month == date.month && candidate.date.day == date.day) {
                aggregate = &candidate;
                break;
            }
        }

        if (!aggregate) {
            if (aggregates.size() == 5) {
                continue;
            }
            aggregates.push_back(ForecastAggregate{});
            aggregate = &aggregates.back();
            aggregate->date = date;
            aggregate->minTemp = minTemp;
            aggregate->maxTemp = maxTemp;
            aggregate->icon = icon;
            aggregate->bestHourDistance = middayDistance;
            aggregate->initialized = true;
            continue;
        }

        aggregate->minTemp = std::min(aggregate->minTemp, minTemp);
        aggregate->maxTemp = std::max(aggregate->maxTemp, maxTemp);
        if (middayDistance < aggregate->bestHourDistance) {
            aggregate->bestHourDistance = middayDistance;
            aggregate->icon = icon;
        }
    }

    forecast.reserve(aggregates.size());
    for (const auto& aggregate : aggregates) {
        if (!aggregate.initialized) {
            continue;
        }
        Forecast entry{aggregate.date, aggregate.minTemp, aggregate.maxTemp, aggregate.icon};
        entry.sunrise = citySunrise;
        entry.sunset = citySunset;
        forecast.push_back(entry);
    }

    char summary[48];
    std::snprintf(summary, sizeof(summary), "Forecast days parsed=%u", static_cast<unsigned>(forecast.size()));
    Logger::info("WeatherParser", summary);

    return forecast;
}
