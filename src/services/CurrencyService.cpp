#include "services/CurrencyService.h"

CurrencyService::CurrencyService(CurrencyRepository& repository) noexcept
    : repository_(repository)
{
}

std::vector<CurrencyRate> CurrencyService::getCurrencies() const
{
    return repository_.fetchCurrencies();
}
