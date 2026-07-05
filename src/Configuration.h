#pragma once

#include <cstdint>
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

    // Interwały odświeżania poszczególnych widgetów (konfigurowalne).
    uint16_t getClockRefreshMinutes() const noexcept;
    uint16_t getWeatherRefreshMinutes() const noexcept;
    uint16_t getCurrencyRefreshMinutes() const noexcept;

    const std::string& getWifiSSID() const noexcept;
    const std::string& getWifiPassword() const noexcept;
    const std::string& getOpenWeatherApiKey() const noexcept;
    float getLatitude() const noexcept;
    float getLongitude() const noexcept;
    const std::string& getLanguage() const noexcept;
    const std::string& getUnits() const noexcept;

private:
    uint16_t clockRefreshMinutes_ = 1;     // zegar odświeża się co 1 minutę
    uint16_t weatherRefreshMinutes_ = 60;  // pogoda i prognoza co 1 godzinę
    uint16_t currencyRefreshMinutes_ = 360; // waluty co 6 godzin
    std::string wifiSSID_;
    std::string wifiPassword_;
    std::string openWeatherApiKey_;
    float latitude_ = 0.0f;
    float longitude_ = 0.0f;
    std::string language_;
    std::string units_;
};
