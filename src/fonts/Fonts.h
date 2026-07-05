#pragma once

#include <cstdint>
#include "BitmapFont.h"

// Rejestr rodzin fontów. Do firmware kompilowana jest tylko JEDNA wybrana rodzina
// (oszczędność Flash) — wybór kompilacyjny, bez przełączania w runtime.
//
// Wybór rodziny (platformio.ini):
//   -D FONT_FAMILY=Roboto         (klasyczny, neutralny — „domyślny” wygląd)
//   -D FONT_FAMILY=Inter
//   -D FONT_FAMILY=JetBrainsMono
//   -D FONT_FAMILY=Atkinson
//
// Renderowanie tekstu (Display.cpp) pobiera font przez bodyFont()/clockFont()/temperatureFont().
namespace fonts
{

enum class Family : uint8_t
{
    Inter = 0,
    JetBrainsMono,
    Atkinson,
    Roboto,
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
