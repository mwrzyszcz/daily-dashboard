#include "Logger.h"
#include "repositories/CurrencyRepository.h"
#include <array>
#include <cstdio>
#include <ctime>

namespace
{
// Kolejność wyświetlania na widgecie: EUR, dolar (USD), frank szwajcarski (CHF).
const std::array<const char*, 3> kCurrencyOrder = {"EUR", "USD", "CHF"};

// Formatuje datę oddaloną o monthsBack miesięcy od podanej jako YYYY-MM-DD.
// Dzień ograniczony do 28, aby uniknąć przeskoków między miesiącami (np. 31 -> kolejny msc).
std::string dateMonthsBack(const Date& date, int monthsBack) noexcept
{
    tm timeInfo = {};
    timeInfo.tm_year = static_cast<int>(date.year) - 1900;
    timeInfo.tm_mon = static_cast<int>(date.month) - 1 - monthsBack;
    timeInfo.tm_mday = date.day < 28 ? date.day : 28;
    if (mktime(&timeInfo) == static_cast<time_t>(-1)) {
        return std::string();
    }

    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d",
                  timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday);
    return std::string(buffer);
}

// Surowe kursy API to "jednostek waluty za 1 PLN"; wyświetlamy odwrotność (PLN za 1 jednostkę).
float toPlnPerUnit(float rawRate) noexcept
{
    return rawRate > 0.0f ? 1.0f / rawRate : 0.0f;
}

// Procentowa zmiana wartości bieżącej względem historycznej.
float percentChange(float current, float past) noexcept
{
    return past > 0.0f ? (current - past) / past * 100.0f : 0.0f;
}
}

CurrencyRepository::CurrencyRepository(const IClockService& clock,
                                       CurrencyClient& client,
                                       CurrencyParser& parser) noexcept
    : clock_(clock)
    , client_(client)
    , parser_(parser)
{
}

std::vector<CurrencyRate> CurrencyRepository::fetchCurrencies() const
{
    Logger::info("CurrencyRepo", "Fetching currency rates");

    const std::map<std::string, float> latest = parser_.parseRates(client_.getLatestJson());

    const Date today = clock_.getDate();
    std::map<std::string, float> past1m;
    std::map<std::string, float> past6m;

    const std::string date1m = dateMonthsBack(today, 1);
    if (!date1m.empty()) {
        past1m = parser_.parseRates(client_.getHistoricalJson(date1m));
    }
    const std::string date6m = dateMonthsBack(today, 6);
    if (!date6m.empty()) {
        past6m = parser_.parseRates(client_.getHistoricalJson(date6m));
    }

    std::vector<CurrencyRate> result;
    result.reserve(kCurrencyOrder.size());

    for (const char* code : kCurrencyOrder) {
        CurrencyRate entry;
        entry.code = code;

        const auto latestIt = latest.find(code);
        if (latestIt == latest.end() || latestIt->second <= 0.0f) {
            result.push_back(entry);
            continue;
        }

        const float plnNow = toPlnPerUnit(latestIt->second);
        entry.rate = plnNow;
        entry.valid = true;

        const auto it1m = past1m.find(code);
        if (it1m != past1m.end() && it1m->second > 0.0f) {
            entry.change1m = percentChange(plnNow, toPlnPerUnit(it1m->second));
            entry.has1m = true;
        }

        const auto it6m = past6m.find(code);
        if (it6m != past6m.end() && it6m->second > 0.0f) {
            entry.change6m = percentChange(plnNow, toPlnPerUnit(it6m->second));
            entry.has6m = true;
        }

        result.push_back(entry);
    }

    return result;
}
