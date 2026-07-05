#pragma once

#include <string>
#include <vector>

/**
 * @brief Kurs jednej waluty względem PLN wraz z procentowymi zmianami.
 *
 * rate to liczba złotych za 1 jednostkę waluty (np. 1 EUR = 4.30 zł).
 * Zmiany procentowe liczone są na wartości wyświetlanej (PLN za jednostkę):
 * dodatnia = waluta zdrożała względem PLN (strzałka w górę).
 */
struct CurrencyRate
{
    std::string code;          // Kod ISO waluty, np. "EUR"
    float rate = 0.0f;         // Aktualny kurs: PLN za 1 jednostkę waluty
    float change1m = 0.0f;     // Procentowa zmiana względem stanu sprzed 1 miesiąca
    float change6m = 0.0f;     // Procentowa zmiana względem stanu sprzed 6 miesięcy
    bool has1m = false;        // Czy dostępne dane historyczne (1 msc)
    bool has6m = false;        // Czy dostępne dane historyczne (6 msc)
    bool valid = false;        // Czy aktualny kurs został pobrany
};
