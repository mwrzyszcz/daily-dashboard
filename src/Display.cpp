#include "Display.h"
#include "Dashboard.h"
#include "Logger.h"
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <vector>

// Domyślna konfiguracja dla Waveshare 7.5" BW na ESP32 Dev Module.
// Jeśli Twój panel ma inny kontroler, podmień DISPLAY_MODEL_CLASS w build_flags.
#ifndef DISPLAY_MODEL_CLASS
#define DISPLAY_MODEL_CLASS GxEPD2_750_T7
#endif

#ifndef DISPLAY_PIN_CS
#define DISPLAY_PIN_CS 5
#endif

#ifndef DISPLAY_PIN_DC
#define DISPLAY_PIN_DC 17
#endif

#ifndef DISPLAY_PIN_RST
#define DISPLAY_PIN_RST 16
#endif

#ifndef DISPLAY_PIN_BUSY
#define DISPLAY_PIN_BUSY 4
#endif

#ifndef DISPLAY_PIN_PWR
#define DISPLAY_PIN_PWR -1
#endif

#ifndef DISPLAY_PIN_SCK
#define DISPLAY_PIN_SCK 18
#endif

#ifndef DISPLAY_PIN_MISO
#define DISPLAY_PIN_MISO 19
#endif

#ifndef DISPLAY_PIN_MOSI
#define DISPLAY_PIN_MOSI 23
#endif

#ifndef DISPLAY_SPI_FREQUENCY
#define DISPLAY_SPI_FREQUENCY 4000000
#endif

#ifndef DISPLAY_RESET_DURATION_MS
#define DISPLAY_RESET_DURATION_MS 10
#endif

#ifndef DISPLAY_HAS_BUSY_CALLBACK
#define DISPLAY_HAS_BUSY_CALLBACK false
#endif

namespace
{
GxEPD2_BW<DISPLAY_MODEL_CLASS, DISPLAY_MODEL_CLASS::HEIGHT> display(
    DISPLAY_MODEL_CLASS(DISPLAY_PIN_CS, DISPLAY_PIN_DC, DISPLAY_PIN_RST, DISPLAY_PIN_BUSY));

// Wysokiej jakości renderer fontów U8g2 na buforze GxEPD2 — pełna obsługa polskich znaków (UTF-8),
// ostre natywne rozmiary bez skalowania. To standardowe rozwiązanie fontowe dla GxEPD2.
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

// =====================================================================================
//  Skala tekstu 
// =====================================================================================
constexpr float kTextScaleTiny  = 0.36f; // etykiety kalendarza, drobne opisy (~12 px)
constexpr float kTextScaleSmall = 0.44f; // etykiety, opisy (~15 px)
constexpr float kTextScaleBody  = 0.40f; // nagłówki sekcji (~18 px)
constexpr float kTextScaleValue = 0.50f; // wyróżnione wartości (~20 px)
constexpr float kTextScaleName  = 0.66f; // imieniny (~22 px)
constexpr float kTextScaleTemp  = 0.18f; // temperatura bieżąca (~25 px)
constexpr float kTextScaleRow   = 0.64f; // dolny wiersz zegara (~15 px)

// =====================================================================================
//  Warstwa dekodowania UTF-8 i rastrowego renderowania fontu z polskimi znakami.
// =====================================================================================

// Odczytuje kolejny kod Unicode ze strumienia UTF-8, aby obsłużyć polskie znaki.
uint32_t nextUtf8Codepoint(const char*& cursor) noexcept
{
    const uint8_t lead = static_cast<uint8_t>(*cursor++);
    if (lead < 0x80) {
        return lead;
    }
    if ((lead & 0xE0) == 0xC0 && *cursor != '\0') {
        const uint8_t tail = static_cast<uint8_t>(*cursor++);
        return ((lead & 0x1F) << 6) | (tail & 0x3F);
    }
    if ((lead & 0xF0) == 0xE0 && cursor[0] != '\0' && cursor[1] != '\0') {
        const uint8_t tail1 = static_cast<uint8_t>(*cursor++);
        const uint8_t tail2 = static_cast<uint8_t>(*cursor++);
        return ((lead & 0x0F) << 12) | ((tail1 & 0x3F) << 6) | (tail2 & 0x3F);
    }
    return '?';
}

// Liczy glify UTF-8 (a nie bajty), co stabilizuje centrowanie i przycinanie tekstu.
size_t utf8Length(const std::string& text) noexcept
{
    size_t length = 0;
    const char* cursor = text.c_str();
    while (*cursor != '\0') {
        nextUtf8Codepoint(cursor);
        ++length;
    }
    return length;
}

// Wysokość odniesienia (bazowa skala 34 px) — do mapowania współczynnika skali na natywny font U8g2.
constexpr float kReferenceCellHeight = 34.0f;

// Dobiera natywny font U8g2 (ostry, z pełnym zestawem polskich znaków) najbliższy docelowej skali.
const uint8_t* fontForScale(const float textScale) noexcept
{
    const int16_t targetHeight = static_cast<int16_t>(std::lround(kReferenceCellHeight * textScale));
    if (targetHeight <= 13) return u8g2_font_helvR10_te; // etykiety, drobne opisy
    if (targetHeight <= 16) return u8g2_font_helvR12_te; // opisy
    if (targetHeight <= 18) return u8g2_font_helvB12_te; // nagłówki sekcji (pogrubione)
    if (targetHeight <= 21) return u8g2_font_helvB14_te; // wyróżnione wartości
    if (targetHeight <= 24) return u8g2_font_helvB18_te; // imieniny
    return u8g2_font_helvB24_te;                          // temperatura bieżąca
}

// Wysokość pojedynczego wiersza tekstu (ascent + descent) dla zadanej skali.
int16_t glyphHeightAt(const float textScale) noexcept
{
    u8g2Fonts.setFont(fontForScale(textScale));
    return static_cast<int16_t>(u8g2Fonts.getFontAscent() - u8g2Fonts.getFontDescent());
}

// Ascent (odległość od linii bazowej do góry glifu) — do wyrównywania tekstów o różnych rozmiarach.
int16_t glyphAscentAt(const float textScale) noexcept
{
    u8g2Fonts.setFont(fontForScale(textScale));
    return static_cast<int16_t>(u8g2Fonts.getFontAscent());
}

// Zwraca szerokość napisu w pikselach dla zadanej skali tekstu (proporcjonalny font U8g2).
int16_t textWidth(const std::string& text, const float textScale) noexcept
{
    u8g2Fonts.setFont(fontForScale(textScale));
    return u8g2Fonts.getUTF8Width(text.c_str());
}

// Rysuje napis od lewej krawędzi (originX) z górną krawędzią w topY, natywnym fontem U8g2 (ostro, UTF-8).
void drawText(const int16_t originX,
              const int16_t topY,
              const std::string& text,
              const float textScale,
              const uint16_t color = GxEPD_BLACK) noexcept
{
    u8g2Fonts.setFont(fontForScale(textScale));
    u8g2Fonts.setFontMode(1); // setFont() resetuje tryb na solid — wymuś przezroczystość, inaczej glify to czarne prostokąty
    u8g2Fonts.setForegroundColor(color);
    u8g2Fonts.drawUTF8(originX, static_cast<int16_t>(topY + u8g2Fonts.getFontAscent()), text.c_str());
}

// Rysuje napis wyśrodkowany w prostokącie o zadanej szerokości.
void drawTextCentered(const int16_t regionX,
                      const int16_t regionWidth,
                      const int16_t topY,
                      const std::string& text,
                      const float textScale,
                      const uint16_t color = GxEPD_BLACK) noexcept
{
    const int16_t startX = regionX + std::max<int16_t>(0, (regionWidth - textWidth(text, textScale)) / 2);
    drawText(startX, topY, text, textScale, color);
}

// Rysuje napis wyrównany do prawej krawędzi (rightX).
void drawTextRightAligned(const int16_t rightX,
                          const int16_t topY,
                          const std::string& text,
                          const float textScale,
                          const uint16_t color = GxEPD_BLACK) noexcept
{
    drawText(rightX - textWidth(text, textScale), topY, text, textScale, color);
}

// Skraca napis do maxGlyphs glifów UTF-8, dopinając wielokropek tylko raz.
std::string truncateWithEllipsis(const std::string& text, const size_t maxGlyphs) noexcept
{
    if (utf8Length(text) <= maxGlyphs) {
        return text;
    }
    if (maxGlyphs == 0) {
        return "";
    }
    const size_t visibleGlyphs = maxGlyphs <= 3 ? maxGlyphs : maxGlyphs - 3;
    std::string result;
    result.reserve(text.size());
    const char* cursor = text.c_str();
    size_t count = 0;
    while (*cursor != '\0' && count < visibleGlyphs) {
        const char* glyphStart = cursor;
        nextUtf8Codepoint(cursor);
        result.append(glyphStart, static_cast<size_t>(cursor - glyphStart));
        ++count;
    }
    if (maxGlyphs > 3) {
        result += "...";
    }
    return result;
}

// Dopasowuje napis do zadanej szerokości, iteracyjnie skracając go z wielokropkiem.
std::string fitText(const std::string& text, const int16_t maxWidth, const float textScale) noexcept
{
    if (maxWidth <= 0) {
        return "";
    }
    if (textWidth(text, textScale) <= maxWidth) {
        return text;
    }
    for (size_t limit = utf8Length(text); limit > 0; --limit) {
        const std::string candidate = truncateWithEllipsis(text, limit);
        if (textWidth(candidate, textScale) <= maxWidth) {
            return candidate;
        }
    }
    return "";
}

// Łamie tekst na słowa i rysuje go wyśrodkowany w kilku wierszach; zwraca Y pod ostatnim wierszem.
int16_t drawWrappedTextCentered(const int16_t regionX,
                                const int16_t regionWidth,
                                const int16_t topY,
                                const std::string& text,
                                const float textScale,
                                const int16_t lineHeight,
                                const size_t maxLines,
                                const uint16_t color = GxEPD_BLACK) noexcept
{
    std::vector<std::string> lines;
    std::string current;
    std::string word;
    std::string source = text + " ";
    for (const char character : source) {
        if (character == ' ') {
            if (!word.empty()) {
                const std::string candidate = current.empty() ? word : current + " " + word;
                if (current.empty() || textWidth(candidate, textScale) <= regionWidth) {
                    current = candidate;
                } else {
                    lines.push_back(current);
                    current = word;
                }
                word.clear();
            }
        } else {
            word += character;
        }
    }
    if (!current.empty()) {
        lines.push_back(current);
    }

    if (lines.empty()) {
        return topY;
    }
    if (lines.size() > maxLines) {
        std::string tail = lines[maxLines - 1];
        for (size_t index = maxLines; index < lines.size(); ++index) {
            tail += " " + lines[index];
        }
        lines.resize(maxLines);
        lines[maxLines - 1] = fitText(tail, regionWidth, textScale);
    }

    int16_t lineTop = topY;
    for (const std::string& line : lines) {
        drawTextCentered(regionX, regionWidth, lineTop, line, textScale, color);
        lineTop += lineHeight;
    }
    return lineTop;
}

// Wielki font zegara — czytelny, natywny rozmiar U8g2 (cyfry i dwukropek), ostre krawędzie.
const uint8_t* const kClockFont = u8g2_font_logisoso78_tn;

// Rysuje wielki zegar natywnym fontem U8g2 (bez skalowania, ostre krawędzie).
void drawClockDigits(const int16_t originX, const int16_t topY, const std::string& text) noexcept
{
    u8g2Fonts.setFont(kClockFont);
    u8g2Fonts.setFontMode(1); // setFont() resetuje tryb na solid — wymuś przezroczystość, inaczej cyfry to czarne prostokąty
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.drawUTF8(originX, static_cast<int16_t>(topY + u8g2Fonts.getFontAscent()), text.c_str());
}

// Szerokość napisu zegara w pikselach.
int16_t clockDigitsWidth(const std::string& text) noexcept
{
    u8g2Fonts.setFont(kClockFont);
    return u8g2Fonts.getUTF8Width(text.c_str());
}

// Dedykowany, powiększony font wskaźnika aktualnej temperatury (cyfry, znak stopnia i litery).
const uint8_t* const kTemperatureFont = u8g2_font_fub30_tf;  // prognoza (kafelki dni)
const uint8_t* const kCurrentTempFont = u8g2_font_fub35_tf;  // aktualna pogoda (jeszcze większa)

// Rysuje wyróżnioną, dużą temperaturę bieżącą natywnym fontem U8g2 (ostro, UTF-8).
void drawTemperatureText(const int16_t originX, const int16_t topY, const std::string& text,
                         const uint8_t* font = kTemperatureFont) noexcept
{
    u8g2Fonts.setFont(font);
    u8g2Fonts.setFontMode(1); // setFont() resetuje tryb na solid — wymuś przezroczystość
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.drawUTF8(originX, static_cast<int16_t>(topY + u8g2Fonts.getFontAscent()), text.c_str());
}

// Szerokość napisu temperatury w pikselach.
int16_t temperatureTextWidth(const std::string& text, const uint8_t* font = kTemperatureFont) noexcept
{
    u8g2Fonts.setFont(font);
    return u8g2Fonts.getUTF8Width(text.c_str());
}

// Ascent dużego fontu temperatury — do wyrównania z mniejszym tekstem do wspólnej linii bazowej.
int16_t temperatureAscent(const uint8_t* font = kTemperatureFont) noexcept
{
    u8g2Fonts.setFont(font);
    return static_cast<int16_t>(u8g2Fonts.getFontAscent());
}

// =====================================================================================
//  Nazewnictwo dat i etykiet po polsku.
// =====================================================================================

// Pełna nazwa dnia tygodnia (1 = poniedziałek .. 7 = niedziela).
const char* fullWeekdayName(const uint8_t weekday) noexcept
{
    static const char* names[] = {
        "", "Poniedziałek", "Wtorek", "Środa", "Czwartek", "Piątek", "Sobota", "Niedziela"
    };
    return weekday >= 1 && weekday <= 7 ? names[weekday] : "";
}

// Dopełniacz nazwy miesiąca do linii daty (np. "24 listopada 2024").
const char* genitiveMonthName(const uint8_t month) noexcept
{
    static const char* names[] = {
        "", "stycznia", "lutego", "marca", "kwietnia", "maja", "czerwca",
        "lipca", "sierpnia", "września", "października", "listopada", "grudnia"
    };
    return month >= 1 && month <= 12 ? names[month] : "";
}

// Nazwa miesiąca wielkimi literami do nagłówka kalendarza (np. "LISTOPAD").
const char* uppercaseMonthName(const uint8_t month) noexcept
{
    static const char* names[] = {
        "KALENDARZ", "STYCZEŃ", "LUTY", "MARZEC", "KWIECIEŃ", "MAJ", "CZERWIEC",
        "LIPIEC", "SIERPIEŃ", "WRZESIEŃ", "PAŹDZIERNIK", "LISTOPAD", "GRUDZIEŃ"
    };
    return month >= 1 && month <= 12 ? names[month] : names[0];
}

// Dwuliterowy skrót dnia tygodnia do nagłówka kalendarza (0 = poniedziałek .. 6 = niedziela).
const char* calendarWeekdayLabel(const uint8_t columnIndex) noexcept
{
    static const char* names[] = { "PN", "WT", "ŚR", "CZ", "PT", "SB", "ND" };
    return columnIndex < 7 ? names[columnIndex] : "--";
}

// Skrót dnia tygodnia do nagłówka prognozy (1 = poniedziałek .. 7 = niedziela).
const char* forecastWeekdayLabel(const uint8_t weekday) noexcept
{
    static const char* names[] = { "", "PON", "WT", "ŚR", "CZW", "PT", "SOB", "NDZ" };
    return weekday >= 1 && weekday <= 7 ? names[weekday] : "--";
}

// Formatuje temperaturę w stopniach Celsjusza ze znakiem stopnia (np. "7°C").
std::string formatTemperature(const float celsius) noexcept
{
    return std::to_string(static_cast<int>(std::lround(celsius))) + "°C";
}

// Formatuje timestamp wschodu/zachodu słońca jako HH:MM.
std::string formatSunTime(const uint32_t unixTimestamp) noexcept
{
    if (unixTimestamp == 0) {
        return "--:--";
    }
    const time_t raw = static_cast<time_t>(unixTimestamp);
    struct tm timeInfo = {};
    if (localtime_r(&raw, &timeInfo) == nullptr) {
        return "--:--";
    }
    char buffer[8];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", timeInfo.tm_hour, timeInfo.tm_min);
    return buffer;
}

// =====================================================================================
//  Ikony rysowane z prymitywów geometrycznych (styl e-paper, grube linie).
// =====================================================================================

// Ramka sekcji z zaokrąglonymi rogami.
void drawSectionFrame(const int16_t x, const int16_t y, const int16_t width, const int16_t height) noexcept
{
    display.drawRoundRect(x, y, width, height, 10, GxEPD_BLACK);
}

// Ikona zegara: okrąg ze wskazówkami.
void drawClockIcon(const int16_t originX, const int16_t originY, const int16_t iconSize) noexcept
{
    const int16_t centerX = originX + iconSize / 2;
    const int16_t centerY = originY + iconSize / 2;
    const int16_t radius = iconSize / 2 - 1;
    display.drawCircle(centerX, centerY, radius, GxEPD_BLACK);
    display.drawLine(centerX, centerY, centerX, centerY - radius + 3, GxEPD_BLACK);
    display.drawLine(centerX, centerY, centerX + radius - 4, centerY + 1, GxEPD_BLACK);
}

// Ikona prezentu do sekcji imienin.
void drawGiftIcon(const int16_t originX, const int16_t originY, const int16_t iconSize) noexcept
{
    const int16_t lidHeight = std::max<int16_t>(4, iconSize / 5);
    const int16_t boxTop = originY + lidHeight + 2;
    const int16_t boxHeight = iconSize - lidHeight - 2;
    const int16_t centerX = originX + iconSize / 2;
    display.drawRect(originX, boxTop, iconSize, boxHeight, GxEPD_BLACK);
    display.drawRect(originX - 1, originY + 2, iconSize + 2, lidHeight, GxEPD_BLACK);
    display.drawFastVLine(centerX, boxTop, boxHeight, GxEPD_BLACK);
    display.drawLine(centerX, originY + 2, centerX - iconSize / 4, originY - 2, GxEPD_BLACK);
    display.drawLine(centerX, originY + 2, centerX + iconSize / 4, originY - 2, GxEPD_BLACK);
}

// Ikona termometru do wartości "odczuwalna".
void drawThermometerIcon(const int16_t originX, const int16_t originY, const int16_t iconSize) noexcept
{
    const int16_t centerX = originX + iconSize / 2;
    display.drawRoundRect(centerX - 3, originY, 6, iconSize - 6, 3, GxEPD_BLACK);
    display.fillCircle(centerX, originY + iconSize - 5, 5, GxEPD_BLACK);
}

// Ikona kropli do wilgotności.
void drawHumidityIcon(const int16_t originX, const int16_t originY, const int16_t iconSize) noexcept
{
    const int16_t centerX = originX + iconSize / 2;
    const int16_t bottomY = originY + iconSize;
    display.drawLine(centerX, originY, centerX - iconSize / 2, bottomY - iconSize / 3, GxEPD_BLACK);
    display.drawLine(centerX, originY, centerX + iconSize / 2, bottomY - iconSize / 3, GxEPD_BLACK);
    display.drawCircle(centerX, bottomY - iconSize / 3, iconSize / 3, GxEPD_BLACK);
}

// Ikona manometru do ciśnienia.
void drawPressureIcon(const int16_t originX, const int16_t originY, const int16_t iconSize) noexcept
{
    const int16_t centerX = originX + iconSize / 2;
    const int16_t centerY = originY + iconSize / 2;
    const int16_t radius = iconSize / 2 - 1;
    display.drawCircle(centerX, centerY, radius, GxEPD_BLACK);
    display.fillCircle(centerX, centerY, 2, GxEPD_BLACK);
    display.drawLine(centerX, centerY, centerX + radius - 3, centerY - radius / 2, GxEPD_BLACK);
}

// Ikona WiFi: łuki i punkt bazowy.
void drawWifiIcon(const int16_t originX, const int16_t originY, const int16_t iconSize) noexcept
{
    const int16_t centerX = originX + iconSize / 2;
    const int16_t centerY = originY + iconSize / 2 - 2;
    display.drawCircle(centerX, centerY, std::max<int16_t>(2, iconSize / 10), GxEPD_BLACK);
    display.drawCircle(centerX, centerY, std::max<int16_t>(5, iconSize / 4), GxEPD_BLACK);
    display.drawCircle(centerX, centerY, std::max<int16_t>(8, iconSize / 3), GxEPD_BLACK);
    display.fillRect(originX, centerY, iconSize, iconSize / 2 + 4, GxEPD_WHITE);
    display.fillCircle(centerX, originY + iconSize - 3, std::max<int16_t>(2, iconSize / 9), GxEPD_BLACK);
}

// Ikona globusa do statusu aktualizacji pogody.
void drawGlobeIcon(const int16_t originX, const int16_t originY, const int16_t iconSize) noexcept
{
    const int16_t centerX = originX + iconSize / 2;
    const int16_t centerY = originY + iconSize / 2;
    const int16_t radius = iconSize / 2 - 1;
    display.drawCircle(centerX, centerY, radius, GxEPD_BLACK);
    display.drawFastHLine(centerX - radius, centerY, radius * 2, GxEPD_BLACK);
    display.drawFastVLine(centerX, centerY - radius, radius * 2, GxEPD_BLACK);
    display.drawFastVLine(centerX - radius / 2, centerY - radius + 3, radius * 2 - 6, GxEPD_BLACK);
    display.drawFastVLine(centerX + radius / 2, centerY - radius + 3, radius * 2 - 6, GxEPD_BLACK);
}

// Ikona statusu NTP: tarcza z krzyżem i pierścieniem.
void drawNtpIcon(const int16_t originX, const int16_t originY, const int16_t iconSize) noexcept
{
    const int16_t centerX = originX + iconSize / 2;
    const int16_t centerY = originY + iconSize / 2;
    const int16_t outerRadius = std::max<int16_t>(8, iconSize / 3);
    display.drawCircle(centerX, centerY, outerRadius, GxEPD_BLACK);
    display.drawFastHLine(centerX - outerRadius + 2, centerY, outerRadius * 2 - 3, GxEPD_BLACK);
    display.drawFastVLine(centerX, centerY - outerRadius + 2, outerRadius * 2 - 3, GxEPD_BLACK);
    display.drawCircle(centerX, centerY, std::max<int16_t>(3, iconSize / 6), GxEPD_BLACK);
}

// Ikona wschodu słońca: słońce nad horyzontem ze strzałką w górę.
void drawSunriseIcon(const int16_t originX, const int16_t originY, const int16_t iconSize) noexcept
{
    const int16_t centerX = originX + iconSize / 2;
    const int16_t horizonY = originY + iconSize / 2 + 4;
    const int16_t radius = std::max<int16_t>(6, iconSize / 5);
    display.drawFastHLine(originX + 2, horizonY, iconSize - 4, GxEPD_BLACK);
    display.drawCircle(centerX, horizonY, radius, GxEPD_BLACK);
    display.fillRect(originX + 2, horizonY, iconSize - 4, radius + 3, GxEPD_WHITE);
    display.drawLine(centerX, originY + 2, centerX, originY + iconSize / 3, GxEPD_BLACK);
    display.drawLine(centerX, originY + 2, centerX - 4, originY + 6, GxEPD_BLACK);
    display.drawLine(centerX, originY + 2, centerX + 4, originY + 6, GxEPD_BLACK);
}

// Ikona zachodu słońca: słońce nad horyzontem ze strzałką w dół.
void drawSunsetIcon(const int16_t originX, const int16_t originY, const int16_t iconSize) noexcept
{
    const int16_t centerX = originX + iconSize / 2;
    const int16_t horizonY = originY + iconSize / 2 + 4;
    const int16_t radius = std::max<int16_t>(6, iconSize / 5);
    display.drawFastHLine(originX + 2, horizonY, iconSize - 4, GxEPD_BLACK);
    display.drawCircle(centerX, horizonY, radius, GxEPD_BLACK);
    display.fillRect(originX + 2, horizonY, iconSize - 4, radius + 3, GxEPD_WHITE);
    display.drawLine(centerX, originY + 2, centerX, originY + iconSize / 3, GxEPD_BLACK);
    display.drawLine(centerX, originY + iconSize / 3, centerX - 4, originY + iconSize / 3 - 4, GxEPD_BLACK);
    display.drawLine(centerX, originY + iconSize / 3, centerX + 4, originY + iconSize / 3 - 4, GxEPD_BLACK);
}

// Chmura z okręgów i zaokrąglonego prostokąta — element ikon pogodowych.
void drawCloudShape(const int16_t originX, const int16_t originY, const int16_t unit) noexcept
{
    const int16_t left = originX + unit;
    const int16_t top = originY + unit * 2;
    display.drawCircle(left + unit * 2, top + unit, unit, GxEPD_BLACK);
    display.drawCircle(left + unit * 4, top, unit + 2, GxEPD_BLACK);
    display.drawCircle(left + unit * 6, top + unit, unit + 1, GxEPD_BLACK);
    display.drawRoundRect(left, top + unit, unit * 7, unit * 3, unit, GxEPD_BLACK);
}

// Słońce z okręgu i promieni co 45 stopni.
void drawSunShape(const int16_t centerX, const int16_t centerY, const int16_t radius) noexcept
{
    display.drawCircle(centerX, centerY, radius, GxEPD_BLACK);
    for (int angle = 0; angle < 360; angle += 45) {
        const float radians = static_cast<float>(angle) * 3.1415926f / 180.0f;
        const int16_t innerX = static_cast<int16_t>(centerX + std::cos(radians) * (radius + 2));
        const int16_t innerY = static_cast<int16_t>(centerY + std::sin(radians) * (radius + 2));
        const int16_t outerX = static_cast<int16_t>(centerX + std::cos(radians) * (radius + 10));
        const int16_t outerY = static_cast<int16_t>(centerY + std::sin(radians) * (radius + 10));
        display.drawLine(innerX, innerY, outerX, outerY, GxEPD_BLACK);
    }
}

// Ukośne kreski deszczu pod chmurą.
void drawRainStrokes(const int16_t originX, const int16_t originY, const int16_t unit) noexcept
{
    for (int index = 0; index < 3; ++index) {
        const int16_t strokeX = originX + unit * (2 + index * 2);
        display.drawLine(strokeX, originY, strokeX - 4, originY + unit * 2, GxEPD_BLACK);
    }
}

// Składa ikonę pogody z prymitywów zależnie od rodzaju zjawiska.
void drawWeatherIcon(const int16_t originX, const int16_t originY, const int16_t iconSize, const WeatherIcon icon) noexcept
{
    const int16_t unit = std::max<int16_t>(4, iconSize / 10);
    switch (icon) {
    case WeatherIcon::ClearDay:
        drawSunShape(originX + iconSize / 2, originY + iconSize / 2, iconSize / 5);
        break;
    case WeatherIcon::ClearNight:
        display.drawCircle(originX + iconSize / 2, originY + iconSize / 2, iconSize / 5, GxEPD_BLACK);
        display.fillCircle(originX + iconSize / 2 + 8, originY + iconSize / 2 - 4, iconSize / 5, GxEPD_WHITE);
        display.drawCircle(originX + iconSize / 2, originY + iconSize / 2, iconSize / 5, GxEPD_BLACK);
        break;
    case WeatherIcon::FewClouds:
        drawSunShape(originX + iconSize / 3, originY + iconSize / 3, iconSize / 8);
        drawCloudShape(originX + iconSize / 5, originY + iconSize / 3, unit);
        break;
    case WeatherIcon::ScatteredClouds:
    case WeatherIcon::BrokenClouds:
        drawCloudShape(originX + iconSize / 6, originY + iconSize / 3, unit + 1);
        break;
    case WeatherIcon::ShowerRain:
    case WeatherIcon::Rain:
        drawCloudShape(originX + iconSize / 6, originY + iconSize / 4, unit + 1);
        drawRainStrokes(originX + iconSize / 6, originY + iconSize - unit * 3, unit + 1);
        break;
    case WeatherIcon::Thunderstorm:
        drawCloudShape(originX + iconSize / 6, originY + iconSize / 4, unit + 1);
        display.drawLine(originX + iconSize / 2, originY + iconSize / 2, originX + iconSize / 2 - 8, originY + iconSize - 14, GxEPD_BLACK);
        display.drawLine(originX + iconSize / 2 - 8, originY + iconSize - 14, originX + iconSize / 2 + 2, originY + iconSize - 14, GxEPD_BLACK);
        display.drawLine(originX + iconSize / 2 + 2, originY + iconSize - 14, originX + iconSize / 2 - 4, originY + iconSize - 2, GxEPD_BLACK);
        break;
    case WeatherIcon::Snow:
        drawCloudShape(originX + iconSize / 6, originY + iconSize / 4, unit + 1);
        display.drawLine(originX + iconSize / 2 - 10, originY + iconSize - 18, originX + iconSize / 2 + 10, originY + iconSize - 6, GxEPD_BLACK);
        display.drawLine(originX + iconSize / 2 + 10, originY + iconSize - 18, originX + iconSize / 2 - 10, originY + iconSize - 6, GxEPD_BLACK);
        display.drawLine(originX + iconSize / 2, originY + iconSize - 22, originX + iconSize / 2, originY + iconSize - 2, GxEPD_BLACK);
        break;
    case WeatherIcon::Mist:
        for (int line = 0; line < 3; ++line) {
            display.drawFastHLine(originX + 6, originY + iconSize / 3 + line * 10, iconSize - 12, GxEPD_BLACK);
        }
        break;
    default:
        drawCloudShape(originX + iconSize / 6, originY + iconSize / 3, unit);
        break;
    }
}

// =====================================================================================
//  Sekcje dashboardu — każda metoda rysuje jeden panel wizualizacji.
// =====================================================================================

// Górny-lewy panel: wielki zegar oraz wiersz z dniem tygodnia i pełną datą.
void drawClockSection(const int16_t x,
                      const int16_t y,
                      const int16_t width,
                      const int16_t height,
                      const Time& time,
                      const Date& date) noexcept
{
    drawSectionFrame(x, y, width, height);

    char timeText[8];
    std::snprintf(timeText, sizeof(timeText), "%02u:%02u", time.hours, time.minutes);
    const int16_t clockStartX = x + std::max<int16_t>(0, (width - clockDigitsWidth(timeText)) / 2);
    drawClockDigits(clockStartX, y + 20, timeText);

    const int16_t rowTop = y + height - 44;
    const int16_t iconSize = 22;
    drawClockIcon(x + 16, rowTop - 1, iconSize);

    const std::string dateText = std::to_string(date.day) + " " + genitiveMonthName(date.month) + " " + std::to_string(date.year);
    const int16_t dateWidth = textWidth(dateText, kTextScaleRow);
    const int16_t dateStartX = x + width - 14 - dateWidth;
    const int16_t separatorX = dateStartX - 12;
    drawText(dateStartX, rowTop, dateText, kTextScaleRow);
    display.drawFastVLine(separatorX, rowTop - 4, glyphHeightAt(kTextScaleRow) + 8, GxEPD_BLACK);

    const int16_t weekdayX = x + 16 + iconSize + 10;
    const int16_t weekdayMaxWidth = separatorX - weekdayX - 8;
    drawText(weekdayX, rowTop, fitText(fullWeekdayName(date.weekday), weekdayMaxWidth, kTextScaleRow), kTextScaleRow);
}

// Górny-środkowy panel: nagłówek IMIENINY oraz imię/imiona bieżącego dnia (bez "następnych").
void drawNamedaySection(const int16_t x,
                        const int16_t y,
                        const int16_t width,
                        const int16_t height,
                        const std::vector<std::string>& namedays) noexcept
{
    drawSectionFrame(x, y, width, height);
    drawTextCentered(x, width, y + 18, "IMIENINY", kTextScaleBody);

    std::string names;
    for (const std::string& entry : namedays) {
        if (!names.empty()) {
            names += ", ";
        }
        names += entry;
    }
    if (names.empty()) {
        names = "Brak";
    }

    const int16_t iconSize = 30;
    drawGiftIcon(x + width / 2 - iconSize / 2, y + 58, iconSize);
    drawWrappedTextCentered(x + 12, width - 24, y + 104, names, kTextScaleName, 26, 2);
}

// Górny-prawy panel: mini-kalendarz miesiąca z podświetlonym dniem bieżącym.
void drawCalendarSection(const int16_t x,
                         const int16_t y,
                         const int16_t width,
                         const int16_t height,
                         const CalendarMonth& calendarMonth,
                         const Date& date) noexcept
{
    drawSectionFrame(x, y, width, height);

    const uint8_t month = calendarMonth.month != 0 ? calendarMonth.month : date.month;
    const uint16_t year = calendarMonth.year != 0 ? calendarMonth.year : date.year;
    const std::string header = std::string(uppercaseMonthName(month)) + " " + std::to_string(year);
    drawTextCentered(x, width, y + 14, fitText(header, width - 20, kTextScaleBody), kTextScaleBody);

    const int16_t gridX = x + 12;
    const int16_t headerRowTop = y + 46;
    const int16_t cellWidth = static_cast<int16_t>((width - 24) / 7);
    for (uint8_t column = 0; column < 7; ++column) {
        drawTextCentered(gridX + column * cellWidth, cellWidth, headerRowTop, calendarWeekdayLabel(column), kTextScaleTiny);
    }

    const int16_t firstWeekTop = headerRowTop + 20;
    const int16_t rowHeight = 20;
    for (size_t weekIndex = 0; weekIndex < calendarMonth.weeks.size() && weekIndex < 6; ++weekIndex) {
        const CalendarWeek& week = calendarMonth.weeks[weekIndex];
        const int16_t rowTop = firstWeekTop + static_cast<int16_t>(weekIndex) * rowHeight;
        for (uint8_t column = 0; column < 7; ++column) {
            const CalendarDay& day = week.days[column];
            if (day.day == 0) {
                continue;
            }
            const int16_t cellX = gridX + column * cellWidth;
            const std::string label = std::to_string(day.day);
            if (day.isToday) {
                const int16_t frameTop = rowTop - 3;
                const int16_t frameHeight = 19;
                display.drawRoundRect(cellX + 2, frameTop, cellWidth - 4, frameHeight, 3, GxEPD_BLACK);
                display.drawRoundRect(cellX + 3, frameTop + 1, cellWidth - 6, frameHeight - 2, 3, GxEPD_BLACK);
                const int16_t todayTop = frameTop + (frameHeight - glyphAscentAt(kTextScaleValue)) / 2;
                drawTextCentered(cellX, cellWidth, todayTop, label, kTextScaleValue);
            } else {
                drawTextCentered(cellX, cellWidth, rowTop, label, kTextScaleTiny);
            }
        }
    }
}

// Rysuje pojedynczy wiersz metryki pogodowej: ikona, etykieta i wyróżniona wartość.
void drawWeatherMetric(const int16_t x,
                       const int16_t y,
                       const int16_t width,
                       const std::string& label,
                       const std::string& value,
                       void (*iconDrawer)(int16_t, int16_t, int16_t)) noexcept
{
    const int16_t iconSize = 18;
    iconDrawer(x, y, iconSize);
    const int16_t textX = x + iconSize + 8;
    const int16_t textWidthLimit = width - iconSize - 8;
    drawText(textX, y, fitText(label, textWidthLimit, kTextScaleTiny), kTextScaleTiny);
    drawText(textX, y + 16, fitText(value, textWidthLimit, kTextScaleValue), kTextScaleValue);
}

// Środkowy-lewy panel: aktualna pogoda — ikona, temperatura, opis i metryki.
void drawCurrentWeatherSection(const int16_t x,
                               const int16_t y,
                               const int16_t width,
                               const int16_t height,
                               const Weather& weather) noexcept
{
    drawSectionFrame(x, y, width, height);

    const int16_t leftWidth = 224;
    const int16_t dividerX = x + leftWidth;
    display.drawFastVLine(dividerX, y + 16, height - 30, GxEPD_BLACK);

    const int16_t iconSize = 84;
    drawWeatherIcon(x + 12, y + 22, iconSize, weather.icon);

    const int16_t tempX = x + 12 + iconSize + 8;
    const int16_t tempMaxWidth = dividerX - tempX - 6;
    const std::string tempText = formatTemperature(weather.temperature);
    if (temperatureTextWidth(tempText, kCurrentTempFont) <= tempMaxWidth) {
        drawTemperatureText(tempX, y + 46, tempText, kCurrentTempFont);
    } else if (temperatureTextWidth(tempText, kTemperatureFont) <= tempMaxWidth) {
        drawTemperatureText(tempX, y + 52, tempText, kTemperatureFont);
    } else {
        drawText(tempX, y + 60, fitText(tempText, tempMaxWidth, kTextScaleTemp), kTextScaleTemp);
    }

    if (!weather.description.empty()) {
        drawWrappedTextCentered(x + 12, leftWidth - 20, y + 128, weather.description, kTextScaleValue, 20, 2);
    }

    const int16_t metricX = dividerX + 14;
    const int16_t metricWidth = x + width - metricX - 10;
    drawWeatherMetric(metricX, y + 26, metricWidth, "Odczuwalna", formatTemperature(weather.feelsLike), drawThermometerIcon);
    drawWeatherMetric(metricX, y + 90, metricWidth, "Wilgotność", std::to_string(weather.humidity) + "%", drawHumidityIcon);
    drawWeatherMetric(metricX, y + 154, metricWidth, "Ciśnienie", std::to_string(weather.pressure) + " hPa", drawPressureIcon);
}

// Środkowy-prawy panel: prognoza na trzy dni w kolumnach (dzień, data, ikona, zakres temperatur).
void drawForecastSection(const int16_t x,
                         const int16_t y,
                         const int16_t width,
                         const int16_t height,
                         const std::vector<Forecast>& forecast) noexcept
{
    drawSectionFrame(x, y, width, height);

    constexpr int16_t columns = 3;
    const int16_t columnWidth = static_cast<int16_t>((width - 24) / columns);
    const int16_t gridX = x + 12;

    for (int16_t column = 1; column < columns; ++column) {
        display.drawFastVLine(gridX + column * columnWidth, y + 16, height - 30, GxEPD_BLACK);
    }

    const size_t count = std::min<size_t>(columns, forecast.size());
    if (count == 0) {
        drawTextCentered(x, width, y + height / 2 - 8, "BRAK DANYCH", kTextScaleBody);
        return;
    }

    for (size_t index = 0; index < count; ++index) {
        const Forecast& day = forecast[index];
        const int16_t columnX = gridX + static_cast<int16_t>(index) * columnWidth;
        drawTextCentered(columnX + 2, columnWidth - 4, y + 20, forecastWeekdayLabel(day.date.weekday), kTextScaleValue);
        const std::string dateLabel = std::to_string(day.date.day) + "." + std::to_string(day.date.month);
        drawTextCentered(columnX + 2, columnWidth - 4, y + 44, dateLabel, kTextScaleBody);

        const int16_t iconSize = 88;
        drawWeatherIcon(columnX + (columnWidth - iconSize) / 2, y + 50, iconSize, day.icon);

        // Wyróżniona temperatura: maksymalna dużym, dedykowanym fontem; minimalna mniejsza i pogrubiona —
        // obie wyrównane do wspólnej linii bazowej i wyśrodkowane w kolumnie.
        const std::string maxStr = std::to_string(static_cast<int>(std::lround(day.maxTemp))) + "\u00B0";
        const std::string minStr = "/" + std::to_string(static_cast<int>(std::lround(day.minTemp))) + "\u00B0";
        const float minScale = kTextScaleValue;
        const int16_t maxTextWidth = temperatureTextWidth(maxStr);
        const int16_t minTextWidth = textWidth(minStr, minScale);
        const int16_t totalWidth = static_cast<int16_t>(maxTextWidth + minTextWidth);
        const int16_t rangeStartX = columnX + 2 + std::max<int16_t>(0, (columnWidth - 4 - totalWidth) / 2);
        const int16_t rangeTop = y + height - 44;
        const int16_t baseline = static_cast<int16_t>(rangeTop + temperatureAscent());
        drawTemperatureText(rangeStartX, rangeTop, maxStr);
        drawText(static_cast<int16_t>(rangeStartX + maxTextWidth),
                 static_cast<int16_t>(baseline - glyphAscentAt(minScale)), minStr, minScale);
    }
}

// Rysuje pojedynczą pozycję dolnego paska: ikona po lewej, etykieta i wartość w dwóch wierszach.
void drawStatusItem(const int16_t x,
                    const int16_t y,
                    const int16_t width,
                    const std::string& label,
                    const std::string& value,
                    void (*iconDrawer)(int16_t, int16_t, int16_t)) noexcept
{
    const int16_t iconSize = 24;
    iconDrawer(x + 10, y + 10, iconSize);
    const int16_t textX = x + 10 + iconSize + 8;
    const int16_t textWidthLimit = width - (textX - x) - 6;
    drawText(textX, y + 8, fitText(label, textWidthLimit, kTextScaleTiny), kTextScaleTiny);
    drawText(textX, y + 26, fitText(value, textWidthLimit, kTextScaleSmall), kTextScaleSmall);
}

// Dolny pasek statusu: WiFi, aktualizacja pogody, NTP, wschód i zachód słońca (bez temperatury wewnętrznej).
void drawStatusBar(const int16_t x,
                   const int16_t y,
                   const int16_t width,
                   const int16_t height,
                   const DashboardState& state) noexcept
{
    drawSectionFrame(x, y, width, height);

    constexpr int16_t items = 5;
    const int16_t columnWidth = static_cast<int16_t>(width / items);
    for (int16_t column = 1; column < items; ++column) {
        display.drawFastVLine(x + column * columnWidth, y + 8, height - 16, GxEPD_BLACK);
    }

    const bool wifiConnected = WiFi.status() == WL_CONNECTED;
    const std::string wifiValue = wifiConnected ? std::string(WiFi.SSID().c_str()) : std::string("Offline");

    char updateValue[24];
    std::snprintf(updateValue, sizeof(updateValue), "%02u.%02u %02u:%02u",
                  state.weatherUpdatedDate.day, state.weatherUpdatedDate.month,
                  state.weatherUpdatedTime.hours, state.weatherUpdatedTime.minutes);

    const std::string ntpValue = (state.date.year >= 2000) ? "OK" : "BŁĄD";

    // Wschód/zachód słońca pochodzą z danych prognozy (obiekt "city" odpowiedzi OpenWeather).
    const uint32_t sunrise = state.forecast.empty() ? 0U : state.forecast.front().sunrise;
    const uint32_t sunset  = state.forecast.empty() ? 0U : state.forecast.front().sunset;

    drawStatusItem(x, y, columnWidth, "WiFi", wifiValue, drawWifiIcon);
    drawStatusItem(x + columnWidth, y, columnWidth, "Aktualizacja", updateValue, drawGlobeIcon);
    drawStatusItem(x + columnWidth * 2, y, columnWidth, "NTP", ntpValue, drawNtpIcon);
    drawStatusItem(x + columnWidth * 3, y, columnWidth, "Wschód", formatSunTime(sunrise), drawSunriseIcon);
    drawStatusItem(x + columnWidth * 4, y, columnWidth, "Zachód", formatSunTime(sunset), drawSunsetIcon);
}

// Prostokąt jednej sekcji dashboardu (współrzędne ekranu).
struct SectionRect
{
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
};

// Rozmieszczenie wszystkich paneli — liczone raz z geometrii ekranu.
struct DashboardLayout
{
    SectionRect clock;
    SectionRect nameday;
    SectionRect calendar;
    SectionRect weather;
    SectionRect forecast;
    SectionRect status;
};

// Wylicza prostokąty paneli. Używane zarówno przez pełne, jak i częściowe odświeżanie.
DashboardLayout computeLayout() noexcept
{
    const int16_t screenWidth = display.width();
    const int16_t screenHeight = display.height();
    const int16_t margin = 6;
    const int16_t gap = 8;

    const int16_t topY = margin;
    const int16_t topHeight = 196;
    const int16_t middleY = topY + topHeight + gap;
    const int16_t statusHeight = 54;
    const int16_t statusY = screenHeight - statusHeight - margin;
    // Sekcja środkowa (pogoda + prognoza) rozciąga się aż do paska statusu, zostawiając tylko
    // standardowy odstęp (gap) — dzięki temu ramki są większe, a przerwa nad statusami minimalna.
    const int16_t middleHeight = static_cast<int16_t>(statusY - gap - middleY);

    const int16_t clockX = margin;
    const int16_t clockWidth = 356;
    const int16_t namedayX = clockX + clockWidth + gap;
    const int16_t namedayWidth = 190;
    const int16_t calendarX = namedayX + namedayWidth + gap;
    const int16_t calendarWidth = screenWidth - calendarX - margin;

    const int16_t weatherX = margin;
    const int16_t weatherWidth = clockWidth;
    const int16_t forecastX = weatherX + weatherWidth + gap;
    const int16_t forecastWidth = screenWidth - forecastX - margin;

    const int16_t statusX = margin;
    const int16_t statusWidth = screenWidth - margin * 2;

    DashboardLayout layout;
    layout.clock    = {clockX, topY, clockWidth, topHeight};
    layout.nameday  = {namedayX, topY, namedayWidth, topHeight};
    layout.calendar = {calendarX, topY, calendarWidth, topHeight};
    layout.weather  = {weatherX, middleY, weatherWidth, middleHeight};
    layout.forecast = {forecastX, middleY, forecastWidth, middleHeight};
    layout.status   = {statusX, statusY, statusWidth, statusHeight};
    return layout;
}

// Zwraca prostokąt panelu przypisanego do danego widgetu (nullptr, gdy brak sekcji graficznej).
const SectionRect* regionForWidget(const std::string& widgetId, const DashboardLayout& layout) noexcept
{
    if (widgetId == "clock_widget")      return &layout.clock;
    if (widgetId == "nameday_widget")    return &layout.nameday;
    if (widgetId == "calendar_widget")   return &layout.calendar;
    if (widgetId == "weather_widget")    return &layout.weather;
    if (widgetId == "forecast_widget")   return &layout.forecast;
    if (widgetId == "status_bar_widget") return &layout.status;
    return nullptr;
}

// Rysuje pojedynczy panel przypisany do widgetu (bez czyszczenia tła — robi to wywołujący).
void drawWidgetSection(const std::string& widgetId,
                       const DashboardState& state,
                       const DashboardLayout& layout) noexcept
{
    if (widgetId == "clock_widget") {
        drawClockSection(layout.clock.x, layout.clock.y, layout.clock.width, layout.clock.height, state.time, state.date);
    } else if (widgetId == "nameday_widget") {
        drawNamedaySection(layout.nameday.x, layout.nameday.y, layout.nameday.width, layout.nameday.height, state.namedays);
    } else if (widgetId == "calendar_widget") {
        drawCalendarSection(layout.calendar.x, layout.calendar.y, layout.calendar.width, layout.calendar.height, state.calendarMonth, state.date);
    } else if (widgetId == "weather_widget") {
        drawCurrentWeatherSection(layout.weather.x, layout.weather.y, layout.weather.width, layout.weather.height, state.weather);
    } else if (widgetId == "forecast_widget") {
        drawForecastSection(layout.forecast.x, layout.forecast.y, layout.forecast.width, layout.forecast.height, state.forecast);
    } else if (widgetId == "status_bar_widget") {
        drawStatusBar(layout.status.x, layout.status.y, layout.status.width, layout.status.height, state);
    }
}

// Rozkłada cały dashboard na siatkę paneli i deleguje rysowanie do sekcji.
void drawDashboard(const Dashboard& dashboard)
{
    const DashboardState& state = dashboard.getState();

    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setTextWrap(false);
    display.setFont();

    const DashboardLayout layout = computeLayout();

    drawClockSection(layout.clock.x, layout.clock.y, layout.clock.width, layout.clock.height, state.time, state.date);
    drawNamedaySection(layout.nameday.x, layout.nameday.y, layout.nameday.width, layout.nameday.height, state.namedays);
    drawCalendarSection(layout.calendar.x, layout.calendar.y, layout.calendar.width, layout.calendar.height, state.calendarMonth, state.date);
    drawCurrentWeatherSection(layout.weather.x, layout.weather.y, layout.weather.width, layout.weather.height, state.weather);
    drawForecastSection(layout.forecast.x, layout.forecast.y, layout.forecast.width, layout.forecast.height, state.forecast);
    drawStatusBar(layout.status.x, layout.status.y, layout.status.width, layout.status.height, state);
}
} // namespace

// Tworzy obiekt wyświetlacza i loguje wybraną strategię odświeżania e-paperu.
Display::Display() noexcept
{
    Logger::info("Display", "Initialized with GxEPD2 partial update support");
}

// Inicjalizuje zasilanie, SPI i panel EPD, a na końcu rysuje ekran startowy.
void Display::init() noexcept
{
    if (DISPLAY_PIN_PWR >= 0) {
        pinMode(DISPLAY_PIN_PWR, OUTPUT);
        digitalWrite(DISPLAY_PIN_PWR, HIGH);
        delay(20);
        Logger::info("Display", "EPD power enabled");
    } else {
        Logger::warn("Display", "DISPLAY_PIN_PWR=-1; for Waveshare e-Paper Driver HAT Rev 2.3 PWR must be tied to 3.3V or driven HIGH");
    }

    char pinInfo[96];
    std::snprintf(pinInfo,
                  sizeof(pinInfo),
                  "Pins CS=%d DC=%d RST=%d BUSY=%d PWR=%d RST_MS=%d",
                  DISPLAY_PIN_CS,
                  DISPLAY_PIN_DC,
                  DISPLAY_PIN_RST,
                  DISPLAY_PIN_BUSY,
                  DISPLAY_PIN_PWR,
                  DISPLAY_RESET_DURATION_MS);
    Logger::info("Display", pinInfo);

    char spiInfo[96];
    std::snprintf(spiInfo,
                  sizeof(spiInfo),
                  "SPI SCK=%d MISO=%d MOSI=%d FREQ=%d",
                  DISPLAY_PIN_SCK,
                  DISPLAY_PIN_MISO,
                  DISPLAY_PIN_MOSI,
                  DISPLAY_SPI_FREQUENCY);
    Logger::info("Display", spiInfo);

    SPI.begin(DISPLAY_PIN_SCK, DISPLAY_PIN_MISO, DISPLAY_PIN_MOSI, DISPLAY_PIN_CS);
    display.epd2.selectSPI(SPI, SPISettings(DISPLAY_SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
    display.init(0, true, DISPLAY_RESET_DURATION_MS, DISPLAY_HAS_BUSY_CALLBACK);
    display.setRotation(2);
    display.setTextColor(GxEPD_BLACK);

    u8g2Fonts.begin(display);          // podłącz renderer fontów U8g2 do bufora GxEPD2
    u8g2Fonts.setFontMode(1);          // tło przezroczyste — rysuje tylko piksele glifu
    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE); // zabezpieczenie: gdyby tryb solid się włączył, tło jest białe a nie czarne
    char dimensions[64];
    std::snprintf(dimensions, sizeof(dimensions), "Display geometry %dx%d", display.width(), display.height());
    Logger::info("Display", dimensions);
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        drawText(20, 30, "Uruchamianie...", kTextScaleBody);
    } while (display.nextPage());

    Logger::info("Display", "E-ink display initialized");
    needsFullRefresh_ = false;
}

// Wykonuje pełne odświeżenie ekranu przez ponowne narysowanie całego dashboardu.
void Display::render(const Dashboard& dashboard) const noexcept
{
    Logger::info("Display", "Full refresh");

    display.setFullWindow();
    display.firstPage();
    do {
        drawDashboard(dashboard);
    } while (display.nextPage());
}

// Odświeża tylko regiony brudnych widgetów za pomocą częściowego okna EPD (bez migotania całości).
void Display::partialUpdate(const Dashboard& dashboard) noexcept
{
    if (needsFullRefresh_) {
        render(dashboard);
        needsFullRefresh_ = false;
        return;
    }

    const auto& widgets = dashboard.getWidgets();
    const DashboardState& state = dashboard.getState();
    const DashboardLayout layout = computeLayout();

    std::string dirtyList;
    int dirtyCount = 0;
    for (auto* widget : widgets) {
        if (!widget || !widget->isDirty()) {
            continue;
        }

        const SectionRect* region = regionForWidget(widget->getId(), layout);
        if (!region) {
            widget->markClean();
            continue;
        }

        if (dirtyCount > 0) {
            dirtyList += ", ";
        }
        dirtyList += widget->getId();
        ++dirtyCount;

        display.setPartialWindow(region->x, region->y, region->width, region->height);
        display.firstPage();
        do {
            display.fillScreen(GxEPD_WHITE);
            display.setTextColor(GxEPD_BLACK);
            display.setTextWrap(false);
            display.setFont();
            drawWidgetSection(widget->getId(), state, layout);
        } while (display.nextPage());

        widget->markClean();
    }

    if (dirtyCount == 0) {
        Logger::info("Display", "Partial update - no dirty widgets");
        return;
    }

    char buf[256];
    std::snprintf(buf, sizeof(buf), "Partial update [%d]: %s", dirtyCount, dirtyList.c_str());
    Logger::info("Display", buf);
}
