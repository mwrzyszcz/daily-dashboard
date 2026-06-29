#include "Configuration.h"

void Configuration::load() noexcept
{
    refreshIntervalMs_ = 10000UL;
    wifiSSID_ = "mw-iot";
    wifiPassword_ = "100200300";
    openWeatherApiKey_ = "34656f074300206bdf7f57c3f263803a";
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
