#pragma once

#include <string>
#include <vector>

class NamedayParser
{
public:
    NamedayParser() = default;
    ~NamedayParser() = default;

    // Parses API response for today's namedays. Returns list of names or empty on error.
    std::vector<std::string> parseToday(const std::string& json) const;
};
