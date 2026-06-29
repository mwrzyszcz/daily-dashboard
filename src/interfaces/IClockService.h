#pragma once

#include "models/Date.h"
#include "models/Time.h"
#include <string>

/**
 * @brief Interfejs usługi czasu.
 */
class IClockService
{
public:
    virtual ~IClockService() = default;
    virtual void begin() = 0;
    virtual void synchronize() = 0;
    virtual bool isTimeValid() const = 0;

    virtual Date getDate() const = 0;
    virtual Time getTime() const = 0;

    virtual std::string getFormattedDate() const = 0;
    virtual std::string getFormattedTime() const = 0;
};
