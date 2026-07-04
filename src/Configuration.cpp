#include "Configuration.h"

void Configuration::load() noexcept
{
    refreshIntervalMs_ = 10000UL;
    clockRefreshMinutes_ = 1;      // zegar co 1 minutę
    weatherRefreshMinutes_ = 60;   // pogoda i prognoza co 1 godzinę
    wifiSSID_ = "mw-changeme";
    wifiPassword_ = "changeme";
    openWeatherApiKey_ = "changeme";
    latitude_ = 51.2465f;
    longitude_ = 22.5684f;
    language_ = "pl";
    units_ = "metric";
    timezone_ = "Europe/Warsaw";
}

unsigned long Configuration::getRefreshIntervalMs() const noexcept
{
    return refreshIntervalMs_;
}

uint16_t Configuration::getClockRefreshMinutes() const noexcept
{
    return clockRefreshMinutes_;
}

uint16_t Configuration::getWeatherRefreshMinutes() const noexcept
{
    return weatherRefreshMinutes_;
}

const std::string& Configuration::getWifiSSID() const noexcept
{
    return wifiSSID_;
}

const std::string& Configuration::getWifiPassword() const noexcept
{
    return wifiPassword_;
}

const std::string& Configuration::getOpenWeatherApiKey() const noexcept
{
    return openWeatherApiKey_;
}

float Configuration::getLatitude() const noexcept
{
    return latitude_;
}

float Configuration::getLongitude() const noexcept
{
    return longitude_;
}

const std::string& Configuration::getLanguage() const noexcept
{
    return language_;
}

const std::string& Configuration::getUnits() const noexcept
{
    return units_;
}

const std::string& Configuration::getTimezone() const noexcept
{
    return timezone_;
}
