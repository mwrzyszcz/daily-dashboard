#include "Logger.h"
#include "clients/CurrencyClient.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

namespace
{
// Bazą jest PLN, a interesują nas EUR, USD (dolar) i CHF (frank szwajcarski).
constexpr const char* kBaseCurrency = "PLN";
constexpr const char* kQuoteCurrencies = "EUR,USD,CHF";

std::string httpGetJson(const std::string& url)
{
    WiFiClientSecure wifiClient;
    wifiClient.setInsecure();

    HTTPClient http;
    if (!http.begin(wifiClient, url.c_str())) {
        Logger::warn("Currency", "HTTP request initialization failed");
        return std::string();
    }

    const int httpCode = http.GET();
    std::string payload;

    if (httpCode == HTTP_CODE_OK) {
        payload = std::string(http.getString().c_str());
    } else {
        Logger::error("Currency", "HTTP error code: " + std::to_string(httpCode));
    }

    http.end();
    return payload;
}
}

std::string CurrencyClient::getLatestJson() const
{
    Logger::info("Currency", "Downloading latest currency rates");
    std::string url = "https://api.frankfurter.dev/v2/rates?base=";
    url += kBaseCurrency;
    url += "&quotes=";
    url += kQuoteCurrencies;
    return httpGetJson(url);
}

std::string CurrencyClient::getHistoricalJson(const std::string& date) const
{
    Logger::info("Currency", "Downloading historical currency rates for " + date);
    std::string url = "https://api.frankfurter.dev/v2/rates?date=";
    url += date;
    url += "&base=";
    url += kBaseCurrency;
    url += "&quotes=";
    url += kQuoteCurrencies;
    return httpGetJson(url);
}
