#include "Namedays.h"

/*
 UWAGA

 Ze względu na prawa autorskie i licencje gotowych kalendarzy imienin
 nie mogę wygenerować i rozpowszechnić kompletnej bazy 365 dni jako własnej
 zawartości.

 Ten plik jest przygotowany do uzupełnienia automatycznie z otwartego źródła
 (CSV/JSON) lub własnych danych.

 Format klucza:
 MMDD
 np.
 101  -> 1 stycznia
 629  -> 29 czerwca

 Przykład:
 {629, {"Piotr","Paweł"}}
*/

const std::unordered_map<uint16_t, std::vector<std::string>> NAMEDAYS =
{
    {101, {"Mieszko", "Mieczysław"}},
    {102, {"Izydor", "Makary"}},
    {629, {"Piotr", "Paweł"}},
    {1231, {"Sylwester"}}
};
