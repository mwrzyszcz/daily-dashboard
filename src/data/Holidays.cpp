#include "Holidays.h"

// Key = month*100 + day
const std::unordered_map<uint16_t, std::string> HOLIDAYS_FIXED = {
    {101,  {"Nowy Rok"}},
    {106,  {"Święto Trzech Króli"}},
    {501,  {"Święto Pracy"}},
    {503,  {"Święto Konstytucji 3 Maja"}},
    {815,  {"Wniebowzięcie Najświętszej Maryi Panny"}},
    {1101, {"Wszystkich Świętych"}},
    {1111, {"Narodowe Święto Niepodległości"}},
    {1225, {"Boże Narodzenie (1. dzień)"}},
    {1226, {"Boże Narodzenie (2. dzień)"}}
};
