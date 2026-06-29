#pragma once

#include "Dashboard.h"
#include "interfaces/IClockService.h"
#include "scheduler/Scheduler.h"

class ApplicationDiagnostics
{
public:
    ApplicationDiagnostics(const Dashboard& dashboard,
                           const IClockService& clockService,
                           const Scheduler& scheduler) noexcept;

    void run() const noexcept;

private:
    void logResult(const char* checkName, bool passed, const char* details) const noexcept;
    bool hasValidDate(const Date& date) const noexcept;

    const Dashboard& dashboard_;
    const IClockService& clockService_;
    const Scheduler& scheduler_;
};