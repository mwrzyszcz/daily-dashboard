#!/usr/bin/env python3
"""
Generator prerasteryzowanych fontów bitmapowych 1-bit dla ESP32 (Display.cpp).

- Wejście: pliki TTF w tools/fonts/ (statyczne lub variable).
- Wyjście: nagłówki src/fonts/generated/<FAMILY>_<ROLE>.h w formacie fonts::BitmapFont.
- Renderowanie: FreeType w trybie MONO (własny hinting fontu) -> ostre bitmapy 1-bit,
  ten sam format co Adafruit_GFX::drawBitmap. Bez TTF w czasie działania urządzenia.

Uruchomienie (Windows / PIO venv):
  & "C:\\Users\\<user>\\.platformio\\penv\\Scripts\\python.exe" tools\\generate_fonts.py
"""

import os
import freetype

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
FONT_SRC = os.path.join(HERE, "fonts")
OUT_DIR = os.path.join(ROOT, "src", "fonts", "generated")

# ---- Zestawy znaków --------------------------------------------------------
ASCII = "".join(chr(c) for c in range(0x20, 0x7F))          # spacja .. ~
POLISH = "ĄĆĘŁŃÓŚŹŻąćęłńóśźż"
FULL = ASCII + POLISH + "°–—"
TEMP = "0123456789.,:-/ %+°ChkmsСc"                           # temperatura + jednostki (h,km/s)
CLOCK = "0123456789: "


def unique_codepoints(charset):
    return sorted(set(ord(ch) for ch in charset))


# ---- Konfiguracja rodzin i ról --------------------------------------------
# file[weight] -> nazwa pliku TTF; variable=True => waga ustawiana osią 'Weight'.
FAMILIES = {
    "INTER": {
        "variable": True,
        "file": {400: "Inter-var.ttf", 700: "Inter-var.ttf"},
    },
    "JETBRAINSMONO": {
        "variable": True,
        "file": {400: "JetBrainsMono-var.ttf", 700: "JetBrainsMono-var.ttf"},
    },
    "ATKINSON": {
        "variable": False,
        "file": {400: "Atkinson-Regular.ttf", 700: "Atkinson-Bold.ttf"},
    },
    "ROBOTO": {
        "variable": True,
        "file": {400: "Roboto-var.ttf", 700: "Roboto-var.ttf"},
    },
}

# (rola, waga, rozmiar_px, zestaw_znaków)
ROLES = [
    ("LABEL",   400, 15, FULL),
    ("BODY",    400, 17, FULL),
    ("HEADING", 700, 17, FULL),
    ("VALUE",   700, 20, FULL),
    ("NAME",    700, 24, FULL),
    ("TEMP_SM", 700, 30, TEMP),
    ("TEMP_LG", 700, 36, TEMP),
    ("CLOCK",   700, 72, CLOCK),
]

MONO_FLAGS = freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_MONO


def apply_weight(face, wght, size_px):
    """Dla fontów variable ustawia oś 'Weight' (i 'Optical Size' jeśli jest)."""
    try:
        info = face.get_variation_info()
    except Exception:
        return  # font statyczny -> waga wynika z wyboru pliku
    coords = []
    for a in info.axes:
        name = a.name if isinstance(a.name, str) else a.name.decode()
        n = name.lower().replace(" ", "")
        lo, hi, dv = a.minimum, a.maximum, a.default
        if n == "weight":
            coords.append(max(lo, min(hi, float(wght))))
        elif n in ("opticalsize", "opsz"):
            coords.append(max(lo, min(hi, float(size_px))))
        else:
            coords.append(dv)
    face.set_var_design_coords(coords)


def render_glyph(face, codepoint):
    """Zwraca (compact_rows_bytes, width, height, left, top, advance) lub None gdy brak glifu."""
    idx = face.get_char_index(codepoint)
    if idx == 0:
        return None
    face.load_char(chr(codepoint), MONO_FLAGS)
    g = face.glyph
    b = g.bitmap
    width, rows, pitch = b.width, b.rows, b.pitch
    advance = g.advance.x >> 6
    left = g.bitmap_left
    top = g.bitmap_top
    compact_pitch = (width + 7) // 8
    data = bytearray()
    if width > 0 and rows > 0:
        buf = b.buffer
        abs_pitch = abs(pitch)
        for r in range(rows):
            # pitch>0: wiersze od góry; pitch<0: od dołu
            src_row = r if pitch > 0 else (rows - 1 - r)
            start = src_row * abs_pitch
            row = buf[start:start + compact_pitch]
            if len(row) < compact_pitch:
                row = list(row) + [0] * (compact_pitch - len(row))
            data.extend(bytes(row))
    return bytes(data), width, rows, left, top, advance


def c_array(name, data):
    lines = [f"static const uint8_t {name}[] = {{"]
    if not data:
        lines[0] = f"static const uint8_t {name}[1] = {{0}}; // pusty"
        return "\n".join(lines)
    for i in range(0, len(data), 16):
        chunk = ", ".join(f"0x{b:02X}" for b in data[i:i + 16])
        lines.append(f"    {chunk},")
    lines.append("};")
    return "\n".join(lines)


def build_font(family_key, cfg, role, weight, size_px, charset):
    name = f"{family_key}_{role}"
    path = os.path.join(FONT_SRC, cfg["file"][weight])
    face = freetype.Face(path)
    if cfg["variable"]:
        apply_weight(face, weight, size_px)
    face.set_pixel_sizes(0, size_px)

    glyphs = []          # (codepoint, offset, w, h, xoff, yoff, adv)
    bitmap = bytearray()
    ascent = 0
    descent = 0
    for cp in unique_codepoints(charset):
        res = render_glyph(face, cp)
        if res is None:
            continue
        data, w, h, left, top, adv = res
        offset = len(bitmap)
        bitmap.extend(data)
        glyphs.append((cp, offset, w, h, left, top, adv))
        ascent = max(ascent, top)
        descent = max(descent, h - top)

    line_height = face.size.height >> 6
    if line_height <= 0:
        line_height = ascent + descent

    # sanity: pola muszą zmieścić się w typach
    assert len(bitmap) < 65536, f"{name}: bitmapa {len(bitmap)}B przekracza uint16"
    for (cp, off, w, h, left, top, adv) in glyphs:
        assert 0 <= w < 256 and 0 <= h < 256, f"{name}: glif {cp} zbyt duży"
        assert -128 <= left <= 127 and -128 <= top <= 127, f"{name}: offset glifu {cp} poza int8"
        assert 0 <= adv < 256, f"{name}: advance glifu {cp} poza uint8"

    # ---- generacja pliku .h ----
    out = []
    out.append("// Wygenerowano przez tools/generate_fonts.py — nie edytuj ręcznie.")
    out.append(f"// {family_key} / {role}  waga={weight}  rozmiar={size_px}px  glify={len(glyphs)}  bitmapa={len(bitmap)}B")
    out.append("#pragma once")
    out.append('#include "../BitmapFont.h"')
    out.append("")
    out.append("namespace fonts {")
    out.append("")
    out.append(c_array(f"{name}_bitmap", bitmap))
    out.append("")
    out.append(f"static const GlyphInfo {name}_glyphs[] = {{")
    for (cp, off, w, h, left, top, adv) in glyphs:
        out.append(f"    {{ 0x{cp:04X}, {off}, {w}, {h}, {left}, {top}, {adv} }},")
    out.append("};")
    out.append("")
    out.append(
        f"static const BitmapFont {name} = {{ {name}_glyphs, "
        f"{len(glyphs)}, {name}_bitmap, {ascent}, {descent}, {line_height} }};"
    )
    out.append("")
    out.append("} // namespace fonts")
    out.append("")

    with open(os.path.join(OUT_DIR, f"{name}.h"), "w", encoding="utf-8") as f:
        f.write("\n".join(out))

    return name, len(glyphs), len(bitmap), ascent, descent, line_height


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    total = 0
    print(f"Generowanie fontów do: {OUT_DIR}")
    for family_key, cfg in FAMILIES.items():
        for (role, weight, size_px, charset) in ROLES:
            name, ng, nb, asc, desc, lh = build_font(
                family_key, cfg, role, weight, size_px, charset
            )
            total += nb
            print(f"  {name:24} glify={ng:3}  bitmapa={nb:5}B  ascent={asc} descent={desc} lh={lh}")
    print(f"Razem bitmap: {total} B ({total/1024:.1f} KiB)")


if __name__ == "__main__":
    main()
