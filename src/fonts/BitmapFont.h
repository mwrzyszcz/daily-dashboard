#pragma once

#include <cstdint>

// Lekki, prerasteryzowany font bitmapowy 1-bit (offline, bez TTF w runtime).
// Renderowany tą samą ścieżką co ikony MDI: Adafruit_GFX::drawBitmap (tło przezroczyste).
// Pełna obsługa UTF-8 (w tym polskich znaków) po stronie renderera w Display.cpp.
namespace fonts
{

// Pojedynczy glif. Bitmapa: wiersze dopełnione do pełnego bajtu ((width+7)/8),
// bity MSB-first, od góry do dołu — format zgodny z Adafruit_GFX::drawBitmap.
struct GlyphInfo
{
    uint16_t codepoint;    // punkt kodowy Unicode (glify posortowane rosnąco → wyszukiwanie binarne)
    uint16_t bitmapOffset; // przesunięcie w tablicy bitmap fontu
    uint8_t  width;        // szerokość glifu w px (0 = brak bitmapy, np. spacja)
    uint8_t  height;       // wysokość glifu w px
    int8_t   xOffset;      // lewy bearing (bitmap_left)
    int8_t   yOffset;      // bitmap_top: px od linii bazowej w górę do górnego wiersza glifu
    uint8_t  xAdvance;     // przesunięcie kursora w px
};

// Font = tablica glifów + wspólna tablica bitmap + metryki pionowe.
struct BitmapFont
{
    const GlyphInfo* glyphs;
    uint16_t         glyphCount;
    const uint8_t*   bitmap;
    uint8_t          ascent;     // px nad linią bazową (górna krawędź tekstu)
    uint8_t          descent;    // px pod linią bazową
    uint8_t          lineHeight; // naturalny odstęp między wierszami
};

} // namespace fonts
