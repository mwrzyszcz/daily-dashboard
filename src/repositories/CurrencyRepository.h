#pragma once

#include <vector>
#include "clients/CurrencyClient.h"
#include "parsers/CurrencyParser.h"
#include "models/Currency.h"
#include "interfaces/IClockService.h"

class CurrencyRepository
{
public:
    CurrencyRepository(const IClockService& clock,
                       CurrencyClient& client,
                       CurrencyParser& parser) noexcept;
    ~CurrencyRepository() = default;

    // Pobiera aktualne kursy oraz zmiany procentowe (1 i 6 miesięcy) dla EUR, USD, CHF.
    std::vector<CurrencyRate> fetchCurrencies() const;

private:
    const IClockService& clock_;
    CurrencyClient& client_;
    CurrencyParser& parser_;
};
