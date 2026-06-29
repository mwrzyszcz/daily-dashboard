#include "Logger.h"
#include "clients/WeatherClient.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

WeatherClient::WeatherClient(const Configuration& configuration) noexcept
    : configuration_(configuration)
{
}

static std::string buildOpenWeatherUrl(const std::string& endpoint, const Configuration& configuration)
{
    std::string url = "https://api.openweathermap.org/data/2.5/";
    url += endpoint;
    url += "?lat=" + std::to_string(configuration.getLatitude());
    url += "&lon=" + std::to_string(configuration.getLongitude());
    url += "&units=" + configuration.getUnits();
    url += "&lang=" + configuration.getLanguage();
    url += "&appid=" + configuration.getOpenWeatherApiKey();
    return url;
}

static std::string httpGetJson(const std::string& url)
{
    Logger::info("Weather", "Downloading weather data");

    WiFiClientSecure wifiClient;
    wifiClient.setInsecure();

    HTTPClient http;
    http.begin(wifiClient, url.c_str());
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK) {
        Logger::error("Weather", "HTTP error code: " + std::to_string(httpCode));
        http.end();
        return "";
    }

    std::string payload = http.getString().c_str();
    http.end();
    return payload;
}

std::string WeatherClient::getCurrentWeatherJson() const
{
    std::string url = buildOpenWeatherUrl("weather", configuration_);
    return httpGetJson(url);
}

std::string WeatherClient::getForecastJson() const
{
    std::string url = buildOpenWeatherUrl("forecast", configuration_);
    return httpGetJson(url);
}
