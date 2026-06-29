#pragma once

#include "interfaces/IClockService.h"
#include "models/Date.h"
#include "models/Time.h"
#include <atomic>
#include <time.h>

class ClockService : public IClockService
{
public:
    ClockService() = default;
    ~ClockService() override = default;

    void begin() override;
    void synchronize() override;
    bool isTimeValid() const override;

    Date getDate() const override;
    Time getTime() const override;
    std::string getFormattedDate() const override;
    std::string getFormattedTime() const override;

private:
    std::atomic<bool> timeValid_{false};
};
