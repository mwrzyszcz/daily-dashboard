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

    JsonObject namedays = data["namedays"].as<JsonObject>();
    if (namedays.isNull()) {
        return result;
    }

    // The API returns language keys mapped to comma separated names, e.g. { "pl": "Piotr, Paweł" }
    for (JsonPair kv : namedays) {
        if (!kv.value().is<const char*>()) continue;
        const char* names = kv.value().as<const char*>();
        std::string s(names);
        size_t start = 0;
        while (start < s.size()) {
            size_t pos = s.find(',', start);
            std::string token = (pos == std::string::npos) ? s.substr(start) : s.substr(start, pos - start);
            token = trim(token);
            if (!token.empty()) result.push_back(token);
            if (pos == std::string::npos) break;
            start = pos + 1;
        }
    }

    return result;
}
