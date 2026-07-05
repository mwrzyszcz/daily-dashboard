#include "Configuration.h"

void Configuration::load() noexcept
{
    wifiSSID_ = "changeme";
    wifiPassword_ = "changeme";
    openWeatherApiKey_ = "changeme";
    latitude_ = 51.2465f;
    longitude_ = 22.5684f;
    language_ = "pl";
    units_ = "metric";
}

uint16_t Configuration::getClockRefreshMinutes() const noexcept
{
    return clockRefreshMinutes_;
}

uint16_t Configuration::getWeatherRefreshMinutes() const noexcept
{
    return weatherRefreshMinutes_;
}

uint16_t Configuration::getCurrencyRefreshMinutes() const noexcept
{
    return currencyRefreshMinutes_;
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
