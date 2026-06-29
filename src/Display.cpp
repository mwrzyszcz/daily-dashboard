#include "Display.h"
#include "Dashboard.h"
#include "Logger.h"
#include <Arduino.h>
#include <GxEPD2_BW.h>

// Konfiguracja wyświetlacza e-ink (przykład dla GxEPD2)
// Dostosuj piny i model wyświetlacza do Twojego hardwaru
// Domyślnie: 800x480 e-ink display
#if !defined(GxEPD2_DISPLAY_CLASS)
#define GxEPD2_DISPLAY_CLASS GxEPD2_565
#endif

#if !defined(GxEPD2_BW)
// Placeholder - będzie zastąpione faktycznym wyświetlaczem
// GxEPD2_BW<GxEPD2_DISPLAY_CLASS, GxEPD2_DISPLAY_CLASS::HEIGHT> display(GxEPD2_DISPLAY_CLASS(/*SPI*, /*CS=5*/, /*DC=17*/, /*RST=16*/, /*BUSY=4*/));
#endif

Display::Display() noexcept
{
    Logger::info("Display", "Initialized with GxEPD2 partial update support");
}

void Display::init() noexcept
{
    // Inicjalizacja wyświetlacza
    // display.init(115200);  // Rozpakuj to gdy dodasz konkretny display
    // display.setFullWindow();
    // display.firstPage();
    // do {
    //     display.fillScreen(GxEPD_WHITE);
    // } while (display.nextPage());
    
    Logger::info("Display", "E-ink display initialized");
    needsFullRefresh_ = false;
}

void Display::render(const Dashboard& dashboard) const noexcept
{
    const auto& s = dashboard.getState();

    static const char* weekdays[] = {"", "Poniedziałek", "Wtorek", "Środa", "Czwartek", "Piątek", "Sobota", "Niedziela"};
    static const char* months[] = {"", "stycznia", "lutego", "marca", "kwietnia", "maja", "czerwca", "lipca", "sierpnia", "września", "października", "listopada", "grudnia"};

    // PEŁNA AKTUALIZACJA - drukuj wszystko
    Logger::info("Display", "Full refresh");

    // Header
    uint8_t wd = s.date.weekday;
    const char* wdname = (wd >=1 && wd <=7) ? weekdays[wd] : "";
    Logger::info("Display", wdname);

    char buf[64];
    std::snprintf(buf, sizeof(buf), "%u %s %u", s.date.day, months[s.date.month], s.date.year);
    Logger::info("Display", buf);

    std::snprintf(buf, sizeof(buf), "%02u:%02u", s.time.hours, s.time.minutes);
    Logger::info("Display", buf);

    Logger::info("Display", "---------------------------------");
    Logger::info("Display", "Imieniny:");
    if (!s.namedays.empty()) {
        for (const auto& n : s.namedays) {
            Logger::info("Display", n.c_str());
        }
    } else {
        Logger::info("Display", "Brak");
    }

    Logger::info("Display", "---------------------------------");
    Logger::info("Display", "Święto:");
    if (s.holiday.isHoliday) {
        Logger::info("Display", s.holiday.name.c_str());
        if (s.holiday.isPublicHoliday) {
            Logger::info("Display", "[Dzień wolny od pracy]");
        }
    } else {
        Logger::info("Display", "Brak");
    }

    Logger::info("Display", "---------------------------------");

    // Month calendar
    static const char* monthNames[] = {"", "Styczeń", "Luty", "Marzec", "Kwiecień", "Maj", "Czerwiec", "Lipiec", "Sierpień", "Wrzesień", "Październik", "Listopad", "Grudzień"};
    char hdr[64];
    std::snprintf(hdr, sizeof(hdr), "%s %u", monthNames[s.calendarMonth.month], s.calendarMonth.year);
    Logger::info("Display", hdr);
    Logger::info("Display", "Pn Wt Śr Cz Pt So Nd");

    for (const auto& week : s.calendarMonth.weeks) {
        char line[128] = {};
        char* p = line;
        for (size_t i = 0; i < week.days.size(); ++i) {
            const auto& d = week.days[i];
            if (!d.inMonth) {
                std::snprintf(p, 4, "   ");
                p += 3;
            } else {
                std::snprintf(p, 4, "%2u ", d.day);
                p += 3;
            }
        }
        Logger::info("Display", line);
    }
    
    // Pełny refresh na e-ink:
    // display.setFullWindow();
    // display.firstPage();
    // do {
    //     // Rysuj wszystkie widgety
    // } while (display.nextPage());
}

void Display::partialUpdate(const Dashboard& dashboard) noexcept
{
    const auto& s = dashboard.getState();
    
    // Jeśli potrzeba pełnego refresh, wykonaj go
    if (needsFullRefresh_) {
        render(dashboard);
        needsFullRefresh_ = false;
        return;
    }
    
    Logger::info("Display", "Partial update - only dirty widgets");
    
    // CZĘŚCIOWA AKTUALIZACJA - tylko zmienione widgety
    // Drukuj tylko zmienione sekcje na serial (dla e-ink: użyj setPartialWindow)
    
    static const char* weekdays[] = {"", "Poniedziałek", "Wtorek", "Środa", "Czwartek", "Piątek", "Sobota", "Niedziela"};
    static const char* months[] = {"", "stycznia", "lutego", "marca", "kwietnia", "maja", "czerwca", "lipca", "sierpnia", "września", "października", "listopada", "grudnia"};
    
    // Zaktualizuj czas (ClockWidget)
    if (true) {  // Sprawdź czy ClockWidget jest dirty
        uint8_t wd = s.date.weekday;
        const char* wdname = (wd >=1 && wd <=7) ? weekdays[wd] : "";
        
        char buf[128];
        std::snprintf(buf, sizeof(buf), "[PARTIAL] Time: %s %u %s %u %02u:%02u", 
                     wdname, s.date.day, months[s.date.month], s.date.year,
                     s.time.hours, s.time.minutes);
        Logger::info("Display", buf);
        
        // Na e-ink: ustaw okno częściowe dla zegara
        // display.setPartialWindow(x, y, width, height);
        // display.firstPage();
        // do { 
        //     // Rysuj tylko ClockWidget
        // } while (display.nextPage());
    }
    
    // Zaktualizuj pogodę (WeatherWidget, ForecastWidget)
    if (true) {  // Sprawdź czy WeatherWidget/ForecastWidget są dirty
        Logger::info("Display", "[PARTIAL] Weather updated");
        // display.setPartialWindow(weather_x, weather_y, weather_width, weather_height);
        // ...
    }
    
    // Zaktualizuj imieniny (NamedayWidget)
    if (true) {  // Sprawdź czy NamedayWidget jest dirty
        Logger::info("Display", "[PARTIAL] Nameday updated");
        if (!s.namedays.empty()) {
            for (const auto& n : s.namedays) {
                std::string msg = "  - " + n;
                Logger::info("Display", msg.c_str());
            }
        }
        // display.setPartialWindow(nameday_x, nameday_y, nameday_width, nameday_height);
        // ...
    }
}

void Display::renderWidget(const Dashboard& dashboard, const std::string& widgetId) const noexcept
{
    // Helper do renderowania pojedynczego widgetu
    // Użyty przez partialUpdate()
    Logger::info("Display", "Rendering widget");
    
    // display.setPartialWindow(widget.x, widget.y, widget.width, widget.height);
    // display.firstPage();
    // do {
    //     // Rysuj widżet
    // } while (display.nextPage());
}
