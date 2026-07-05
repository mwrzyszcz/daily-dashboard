#pragma once

#include <vector>
#include "models/Currency.h"
#include "repositories/CurrencyRepository.h"

class CurrencyService
{
public:
    explicit CurrencyService(CurrencyRepository& repository) noexcept;
    ~CurrencyService() = default;

    std::vector<CurrencyRate> getCurrencies() const;

private:
    CurrencyRepository& repository_;
};
