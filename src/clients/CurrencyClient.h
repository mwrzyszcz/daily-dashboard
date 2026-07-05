#pragma once

#include <string>

/**
 * @brief Klient REST API frankfurter.dev (kursy walut Europejskiego Banku Centralnego).
 *
 * Pobiera kursy z bazą w PLN dla walut EUR, USD, CHF — aktualne oraz historyczne
 * (do wyliczenia zmian procentowych). Zwraca surowy JSON do sparsowania.
 */
class CurrencyClient
{
public:
    CurrencyClient() = default;
    ~CurrencyClient() = default;

    // Aktualne kursy (base=PLN, quotes=EUR,USD,CHF).
    std::string getLatestJson() const;

    // Kursy z konkretnego dnia (format daty: YYYY-MM-DD).
    std::string getHistoricalJson(const std::string& date) const;
};
