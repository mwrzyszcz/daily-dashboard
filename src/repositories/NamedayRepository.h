#pragma once

#include "models/Date.h"
#include <vector>
#include <string>
#include <optional>

#include "clients/NamedayClient.h"
#include "parsers/NamedayParser.h"
#include "interfaces/IClockService.h"

class NamedayRepository
{
public:
    explicit NamedayRepository(const IClockService& clock) noexcept;
    ~NamedayRepository() = default;

    // Returns namedays for requested date. May trigger download if needed.
    std::vector<std::string> getNamedays(const Date& date);

private:
    bool shouldRefresh(const Date& date) const noexcept;
    void refresh(const Date& date);

    Date lastDownloadDate_{};
    std::vector<std::string> cachedNamedays_{};
    NamedayClient client_{};
    NamedayParser parser_{};
    const IClockService& clock_;
};
