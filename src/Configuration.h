#pragma once

#include <string>

/**
 * @brief Zarządza ustawieniami aplikacji.
 */
class Configuration
{
public:
    Configuration() = default;
    ~Configuration() = default;

    void load() noexcept;

    unsigned long getRefreshIntervalMs() const noexcept;
    const std::string& getWifiSSID() const noexcept;
    const std::string& getWifiPassword() const noexcept;
    const std::string& getOpenWeatherApiKey() const noexcept;
    float getLatitude() const noexcept;
    float getLongitude() const noexcept;
    const std::string& getLanguage() const noexcept;
    const std::string& getUnits() const noexcept;
    const std::string& getTimezone() const noexcept;

private:
    unsigned long refreshIntervalMs_ = 30000UL;
    std::string wifiSSID_;
    std::string wifiPassword_;
    std::string openWeatherApiKey_;
    float latitude_ = 0.0f;
    float longitude_ = 0.0f;
    std::string language_;
    std::string units_;
    std::string timezone_;
};
