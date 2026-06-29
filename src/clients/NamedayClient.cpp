#include "Logger.h"
#include "clients/NamedayClient.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

std::string NamedayClient::downloadToday() const
{
    Logger::info("Nameday", "Downloading today's namedays");
    const std::string url = "https://nameday.abalin.net/api/V2/today?timezone=Europe/Warsaw";

    WiFiClientSecure wifiClient;
    wifiClient.setInsecure();

    HTTPClient http;
    if (!http.begin(wifiClient, url.c_str())) {
        Logger::warn("Nameday", "HTTP request initialization failed");
        return std::string();
    }

    const int httpCode = http.GET();
    std::string payload;

    if (httpCode == HTTP_CODE_OK) {
        const String result = http.getString();
        payload = std::string(result.c_str());
    } else {
        Logger::warn("Nameday", "HTTP request failed");
    }

    http.end();
    return payload;
}
