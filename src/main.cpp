#include <Arduino.h>
#include "Logger.h"
#include "Application.h"
#include "Configuration.h"
#include "Display.h"
#include "Dashboard.h"
#include "layout/VerticalLayout.h"
#include "widgets/ClockWidget.h"
#include "widgets/WeatherWidget.h"
#include "widgets/ForecastWidget.h"
#include "widgets/CalendarWidget.h"
#include "widgets/CurrencyWidget.h"
#include "widgets/NamedayWidget.h"
#include "widgets/HolidayWidget.h"
#include "widgets/StatusBarWidget.h"
#include "services/ClockService.h"
#include "services/WeatherService.h"
#include "services/CalendarService.h"
#include "services/HolidayService.h"
#include "services/NamedayService.h"
#include "services/CurrencyService.h"
#include "repositories/WeatherRepository.h"
#include "clients/WeatherClient.h"
#include "parsers/WeatherParser.h"
#include "clients/NamedayClient.h"
#include "parsers/NamedayParser.h"
#include "repositories/NamedayRepository.h"
#include "clients/CurrencyClient.h"
#include "parsers/CurrencyParser.h"
#include "repositories/CurrencyRepository.h"

static Application& application()
{
    static Configuration configuration;
    configuration.load();

    static WeatherClient weatherClient(configuration);
    static WeatherParser weatherParser;
    static WeatherRepository weatherRepository(configuration, weatherClient, weatherParser);

    static ClockService clockService;
    static WeatherService weatherService(weatherRepository);
    static CalendarService calendarService;
    static HolidayService holidayService;

    // Nameday REST pipeline — create client/parser/repository and service
    static NamedayRepository namedayRepository(clockService);
    static NamedayService namedayService(namedayRepository);

    // Currency REST pipeline (frankfurter.dev) — kursy EUR/USD/CHF względem PLN
    static CurrencyClient currencyClient;
    static CurrencyParser currencyParser;
    static CurrencyRepository currencyRepository(clockService, currencyClient, currencyParser);
    static CurrencyService currencyService(currencyRepository);

    static ClockWidget clockWidget("clock_widget");
    static WeatherWidget weatherWidget("weather_widget");
    static ForecastWidget forecastWidget("forecast_widget");
    static CalendarWidget calendarWidget("calendar_widget");
    static CurrencyWidget currencyWidget("currency_widget");
    static NamedayWidget namedayWidget("nameday_widget");
    static HolidayWidget holidayWidget("holiday_widget");
    static StatusBarWidget statusBarWidget("status_bar_widget");

    static VerticalLayout mainLayout("main_layout");

    static Dashboard dashboard(mainLayout);
    dashboard.addWidget(clockWidget);
    dashboard.addWidget(weatherWidget);
    dashboard.addWidget(forecastWidget);
    // Widget Kalendarz zastąpiony widgetem Waluty. Aby przywrócić kalendarz,
    // odkomentuj poniższą linię i zakomentuj addWidget(currencyWidget).
    // dashboard.addWidget(calendarWidget);
    dashboard.addWidget(currencyWidget);
    dashboard.addWidget(namedayWidget);
    dashboard.addWidget(holidayWidget);
    dashboard.addWidget(statusBarWidget);

    static Display display;
    static Application app(configuration,
                           dashboard,
                           display,
                           clockService,
                           weatherService,
                           calendarService,
                           holidayService,
                           namedayService,
                           currencyService);
    return app;
}

void setup()
{
    Logger::begin(115200);
    Logger::info("Application", "Starting");
    application().setup();
}

void loop()
{
    application().loop();
}
