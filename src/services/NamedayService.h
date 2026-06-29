#pragma once

#include "models/Date.h"
#include <string>
#include <vector>

class NamedayRepository;

class NamedayService
{
public:
    explicit NamedayService(NamedayRepository& repository) noexcept;
    ~NamedayService() = default;

    std::vector<std::string> getNamedays(const Date& date) const;
    bool hasNameday(const Date& date) const;

private:
    NamedayRepository& repository_;
};

