#include "Logger.h"
#include "repositories/NamedayRepository.h"
#include <Arduino.h>

NamedayRepository::NamedayRepository(const IClockService& clock) noexcept
    : client_(), parser_(), clock_(clock)
{
}

bool NamedayRepository::shouldRefresh(const Date& date) const noexcept
{
    if (lastDownloadDate_.year == 0) return true;
    return (lastDownloadDate_.year != date.year) || (lastDownloadDate_.month != date.month) || (lastDownloadDate_.day != date.day);
}

void NamedayRepository::refresh(const Date& date)
{
    Logger::info("Nameday", "Downloading...");
    const std::string payload = client_.downloadToday();
    if (payload.empty()) {
        Logger::warn("Nameday", "Download failed or empty response");
        return;
    }

    const auto names = parser_.parseToday(payload);
    if (names.empty()) {
        Logger::warn("Nameday", "Parser returned empty data");
        return;
    }

    cachedNamedays_ = names;
    lastDownloadDate_ = date;
}

std::vector<std::string> NamedayRepository::getNamedays(const Date& date)
{
    if (shouldRefresh(date)) {
        refresh(date);
        if (!cachedNamedays_.empty()) {
            Logger::info("Nameday", "Loaded from network");
            return cachedNamedays_;
        }
    } else {
        Logger::info("Nameday", "Loaded from cache");
        return cachedNamedays_;
    }

    // If network failed and cache exists, return cache.
    if (!cachedNamedays_.empty()) {
        Logger::warn("Nameday", "Returning stale cache");
        return cachedNamedays_;
    }

    return {};
}
