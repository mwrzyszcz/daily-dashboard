#include "parsers/CurrencyParser.h"
#include "Logger.h"
#include <ArduinoJson.h>

std::map<std::string, float> CurrencyParser::parseRates(const std::string& json) const
{
    std::map<std::string, float> rates;
    if (json.empty()) {
        Logger::warn("CurrencyParser", "Rates payload is empty");
        return rates;
    }

    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, json.c_str());
    if (error) {
        Logger::error("CurrencyParser", std::string("Rates JSON parse failed: ") + error.c_str());
        return rates;
    }

    JsonArray entries = doc.as<JsonArray>();
    if (entries.isNull()) {
        Logger::warn("CurrencyParser", "Rates payload is not an array");
        return rates;
    }

    for (JsonObject entry : entries) {
        const char* quote = entry["quote"] | "";
        const float rate = entry["rate"] | 0.0f;
        if (quote[0] != '\0' && rate > 0.0f) {
            rates[std::string(quote)] = rate;
        }
    }

    return rates;
}
