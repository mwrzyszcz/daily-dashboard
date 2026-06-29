
#include "Logger.h"
#include "NamedayService.h"
#include "repositories/NamedayRepository.h"

NamedayService::NamedayService(NamedayRepository& repository) noexcept
    : repository_(repository)
{
}

std::vector<std::string> NamedayService::getNamedays(const Date& date) const
{
    Logger::info("NamedayService", "Requesting namedays for current date");
    return repository_.getNamedays(date);
}

bool NamedayService::hasNameday(const Date& date) const
{
    auto v = repository_.getNamedays(date);
    return !v.empty();
}


