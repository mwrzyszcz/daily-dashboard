#pragma once

#include <cstdint>
#include "BitmapFont.h"

// Rejestr rodzin fontów. Do firmware kompilowana jest tylko JEDNA wybrana rodzina
// (oszczędność Flash) — wybór kompilacyjny, bez przełączania w runtime.
//
// Wybór rodziny (platformio.ini):
//   -D FONT_FAMILY=Helvetica      (domyślna, klasyczny renderer U8g2 — bez offline rasteryzacji)
//   -D FONT_FAMILY=Roboto         (klasyczny, neutralny)
//   -D FONT_FAMILY=Inter
//   -D FONT_FAMILY=JetBrainsMono
//   -D FONT_FAMILY=Atkinson
//
// Renderowanie tekstu (Display.cpp) pobiera font przez bodyFont()/clockFont()/temperatureFont()
// (rodziny bitmapowe) albo używa wbudowanych fontów U8g2 (gdy FONT_USE_U8G2).

#ifndef FONT_FAMILY
#define FONT_FAMILY Inter
#endif

// Identyfikatory rodzin (do dyrektyw #if) — współdzielone przez Display.cpp i Fonts.cpp.
#define FF_Inter 0
#define FF_JetBrainsMono 1
#define FF_Atkinson 2
#define FF_Roboto 3
#define FF_Helvetica 4
#define FF_PASTE_(a, b) a##b
#define FF_PASTE(a, b) FF_PASTE_(a, b)
#define FONT_FAMILY_ID FF_PASTE(FF_, FONT_FAMILY)

// FONT_FAMILY=Helvetica → klasyczny renderer U8g2 (wbudowane fonty, bez plików src/fonts/generated).
#if FONT_FAMILY_ID == FF_Helvetica
#define FONT_USE_U8G2 1
#endif

namespace fonts
{

enum class Family : uint8_t
{
    Inter = 0,
    JetBrainsMono,
    Atkinson,
    Roboto,
    Helvetica,
    Count
};

// Aktywna rodzina fontów (skompilowana w tym buildzie). setActiveFamily jest no-op —
// kompilowana jest tylko jedna rodzina; aby zmienić font, ustaw -D FONT_FAMILY i przebuduj.
void        setActiveFamily(Family family) noexcept;
Family      activeFamily() noexcept;
const char* familyName(Family family) noexcept;

// Font tekstu interfejsu dobrany do docelowej wysokości w px (etykieta … nagłówek).
const BitmapFont& bodyFont(int targetHeightPx) noexcept;

// Wielki font zegara (cyfry + dwukropek).
const BitmapFont& clockFont() noexcept;

// Font temperatury: current=false → kafelki prognozy, current=true → duża temperatura bieżąca.
const BitmapFont& temperatureFont(bool current) noexcept;

} // namespace fonts
