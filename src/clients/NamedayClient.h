#pragma once

#include <string>

class NamedayClient
{
public:
    NamedayClient() = default;
    ~NamedayClient() = default;

    // Downloads today's namedays JSON from the API and returns raw payload.
    std::string downloadToday() const;
};
