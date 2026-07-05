#pragma once

#include <map>
#include <string>

/**
 * @brief Parser odpowiedzi frankfurter.dev.
 *
 * Odpowiedź v2 to tablica obiektów: [{"date":..,"base":"PLN","quote":"EUR","rate":0.233}, ...].
 * Zwraca mapę kod waluty -> surowy kurs (jednostek waluty za 1 PLN).
 */
class CurrencyParser
{
public:
    CurrencyParser() = default;
    ~CurrencyParser() = default;

    std::map<std::string, float> parseRates(const std::string& json) const;
};
