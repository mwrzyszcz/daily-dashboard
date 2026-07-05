#include "Fonts.h"

// =====================================================================================
//  Kompilacja WARUNKOWA: do firmware trafia tylko JEDNA wybrana rodzina fontów
//  (oszczędność pamięci Flash — pozostałe rodziny nie są w ogóle kompilowane).
//  Wybór przez flagę w platformio.ini:
//
//      -D FONT_FAMILY=Helvetica      (domyślna — klasyczny renderer U8g2, bez offline rasteryzacji)
//      -D FONT_FAMILY=Roboto         (klasyczny, neutralny)
//      -D FONT_FAMILY=Inter
//      -D FONT_FAMILY=JetBrainsMono
//      -D FONT_FAMILY=Atkinson
//
//  Pliki fontów bitmapowych generuje: tools/generate_fonts.py
//  (Helvetica nie korzysta z tych plików — używa wbudowanych fontów U8g2, patrz Display.cpp.)
// =====================================================================================

// Wybór backendu: Helvetica → U8g2 (Display.cpp), pozostałe → bitmapy z src/fonts/generated.
#ifdef FONT_USE_U8G2
#define FONT_FAMILY_ENUM Helvetica
#else

#if FONT_FAMILY_ID == FF_Inter
#define FONT_FAMILY_PREFIX INTER
#define FONT_FAMILY_ENUM Inter
#include "generated/INTER_LABEL.h"
#include "generated/INTER_BODY.h"
#include "generated/INTER_HEADING.h"
#include "generated/INTER_VALUE.h"
#include "generated/INTER_NAME.h"
#include "generated/INTER_TEMP_SM.h"
#include "generated/INTER_TEMP_LG.h"
#include "generated/INTER_CLOCK.h"

#elif FONT_FAMILY_ID == FF_JetBrainsMono
#define FONT_FAMILY_PREFIX JETBRAINSMONO
#define FONT_FAMILY_ENUM JetBrainsMono
#include "generated/JETBRAINSMONO_LABEL.h"
#include "generated/JETBRAINSMONO_BODY.h"
#include "generated/JETBRAINSMONO_HEADING.h"
#include "generated/JETBRAINSMONO_VALUE.h"
#include "generated/JETBRAINSMONO_NAME.h"
#include "generated/JETBRAINSMONO_TEMP_SM.h"
#include "generated/JETBRAINSMONO_TEMP_LG.h"
#include "generated/JETBRAINSMONO_CLOCK.h"

#elif FONT_FAMILY_ID == FF_Atkinson
#define FONT_FAMILY_PREFIX ATKINSON
#define FONT_FAMILY_ENUM Atkinson
#include "generated/ATKINSON_LABEL.h"
#include "generated/ATKINSON_BODY.h"
#include "generated/ATKINSON_HEADING.h"
#include "generated/ATKINSON_VALUE.h"
#include "generated/ATKINSON_NAME.h"
#include "generated/ATKINSON_TEMP_SM.h"
#include "generated/ATKINSON_TEMP_LG.h"
#include "generated/ATKINSON_CLOCK.h"

#elif FONT_FAMILY_ID == FF_Roboto
#define FONT_FAMILY_PREFIX ROBOTO
#define FONT_FAMILY_ENUM Roboto
#include "generated/ROBOTO_LABEL.h"
#include "generated/ROBOTO_BODY.h"
#include "generated/ROBOTO_HEADING.h"
#include "generated/ROBOTO_VALUE.h"
#include "generated/ROBOTO_NAME.h"
#include "generated/ROBOTO_TEMP_SM.h"
#include "generated/ROBOTO_TEMP_LG.h"
#include "generated/ROBOTO_CLOCK.h"

#else
#error "Nieznana FONT_FAMILY — dozwolone: Helvetica | Roboto | Inter | JetBrainsMono | Atkinson"
#endif

// Nazwa obiektu fontu dla roli aktywnej rodziny, np. ROLE_FONT(_CLOCK) -> INTER_CLOCK.
#define ROLE_FONT(role) FF_PASTE(FONT_FAMILY_PREFIX, role)

#endif // FONT_USE_U8G2

namespace fonts
{

#ifndef FONT_USE_U8G2
// Kolejność ról musi odpowiadać ROLES w tools/generate_fonts.py.
enum Role : uint8_t
{
    R_LABEL = 0,
    R_BODY,
    R_HEADING,
    R_VALUE,
    R_NAME,
    R_TEMP_SM,
    R_TEMP_LG,
    R_CLOCK,
    R_COUNT
};

// Tylko wybrana (skompilowana) rodzina — jedna tablica ról.
static const BitmapFont* const kActiveFonts[R_COUNT] = {
    &ROLE_FONT(_LABEL),   &ROLE_FONT(_BODY),    &ROLE_FONT(_HEADING), &ROLE_FONT(_VALUE),
    &ROLE_FONT(_NAME),    &ROLE_FONT(_TEMP_SM), &ROLE_FONT(_TEMP_LG), &ROLE_FONT(_CLOCK),
};
#endif // FONT_USE_U8G2

// Rodzina skompilowana w tym buildzie. Przełączenie w runtime nie jest możliwe —
// pozostałe rodziny nie są dołączone do firmware (oszczędność Flash).
static const Family kCompiledFamily = Family::FONT_FAMILY_ENUM;

void setActiveFamily(Family family) noexcept
{
    // Skompilowana jest tylko jedna rodzina — żądania innych są ignorowane.
    // Aby zmienić font, ustaw -D FONT_FAMILY=... w platformio.ini i przebuduj.
    (void)family;
}

Family activeFamily() noexcept
{
    return kCompiledFamily;
}

const char* familyName(Family family) noexcept
{
    switch (family) {
        case Family::Inter:         return "Inter";
        case Family::JetBrainsMono: return "JetBrains Mono";
        case Family::Atkinson:      return "Atkinson Hyperlegible";
        case Family::Roboto:        return "Roboto";
        case Family::Helvetica:     return "Helvetica (U8g2)";
        default:                    return "?";
    }
}

#ifndef FONT_USE_U8G2
static const BitmapFont& role(Role r) noexcept
{
    return *kActiveFonts[r];
}
const BitmapFont& bodyFont(int targetHeightPx) noexcept
{
    Role r;
    if (targetHeightPx <= 13)      r = R_LABEL;
    else if (targetHeightPx <= 16) r = R_BODY;
    else if (targetHeightPx <= 18) r = R_HEADING;
    else if (targetHeightPx <= 20) r = R_VALUE;
    else                           r = R_NAME;
    return role(r);
}

const BitmapFont& clockFont() noexcept
{
    return role(R_CLOCK);
}

const BitmapFont& temperatureFont(bool current) noexcept
{
    return role(current ? R_TEMP_LG : R_TEMP_SM);
}

#endif // FONT_USE_U8G2

} // namespace fonts
