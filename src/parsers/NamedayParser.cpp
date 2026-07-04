#include "parsers/NamedayParser.h"
#include <ArduinoJson.h>
#include <cctype>

static inline std::string trim(const std::string& s)
{
    size_t a = 0;
    while (a < s.size() && isspace(static_cast<unsigned char>(s[a]))) ++a;
    size_t b = s.size();
    while (b > a && isspace(static_cast<unsigned char>(s[b-1]))) --b;
    return s.substr(a, b - a);
}

static void appendCommaSeparated(std::vector<std::string>& result, const char* names)
{
    if (names == nullptr) {
        return;
    }

    std::string text(names);
    size_t start = 0;
    while (start < text.size()) {
        size_t pos = text.find(',', start);
        std::string token = (pos == std::string::npos) ? text.substr(start) : text.substr(start, pos - start);
        token = trim(token);
        if (!token.empty() && token != "n/a") {
            result.push_back(token);
        }
        if (pos == std::string::npos) {
            break;
        }
        start = pos + 1;
    }
}

std::vector<std::string> NamedayParser::parseToday(const std::string& json) const
{
    std::vector<std::string> result;
    if (json.empty()) {
        return result;
    }

    // Allocate reasonably sized dynamic document
    DynamicJsonDocument doc(2048);
    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        return result;
    }

    JsonObject data = doc["data"].as<JsonObject>();
    if (data.isNull()) {
        return result;
    }

    // Current API format returns language entries directly under data, e.g. data.pl.
    if (data["pl"].is<const char*>()) {
        appendCommaSeparated(result, data["pl"].as<const char*>());
        if (!result.empty()) {
            return result;
        }
    }

    // Backward compatibility for the older data.namedays.pl shape.
    JsonObject namedays = data["namedays"].as<JsonObject>();
    if (namedays.isNull()) {
        return result;
    }

    if (namedays["pl"].is<const char*>()) {
        appendCommaSeparated(result, namedays["pl"].as<const char*>());
        return result;
    }

    // Last-resort fallback if the API doesn't expose a dedicated Polish entry.
    for (JsonPair kv : namedays) {
        if (!kv.value().is<const char*>()) continue;
        appendCommaSeparated(result, kv.value().as<const char*>());
        if (!result.empty()) {
            break;
        }
    }

    return result;
}
