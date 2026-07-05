#!/usr/bin/env python3
"""Generuje nagłówek C++ z ikonami Material Design Icons (MDI) jako bitmapy 1-bit.

Ikony są rasteryzowane z oficjalnego webfontu MDI do formatu zgodnego z
Adafruit_GFX::drawBitmap (wiersze wyrównane do bajtu, MSB-first). Dzięki temu na
1-bitowym e-paperze wyglądają identycznie jak w openHASP, bez konieczności
budowania fontu U8g2 (który i tak jest bitmapą o stałym rozmiarze).

Uruchom raz, aby wygenerować src/generated/MdiIcons.h — nagłówek jest commitowany,
więc kompilacja firmware nie wymaga Pythona ani sieci.
"""

import os
import re
import sys
import urllib.request

from PIL import Image, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
CACHE_DIR = os.path.join(HERE, ".mdi-cache")
OUT_PATH = os.path.join(HERE, "..", "src", "generated", "MdiIcons.h")

TTF_URL = "https://cdn.jsdelivr.net/npm/@mdi/font/fonts/materialdesignicons-webfont.ttf"
CSS_URL = "https://cdn.jsdelivr.net/npm/@mdi/font/css/materialdesignicons.min.css"

# (identyfikator w C++, nazwa ikony MDI, rozmiar w pikselach)
ICONS = [
    # Pogoda (używane w kafelkach bieżącej pogody i prognozy)
    ("WEATHER_SUNNY",          "weather-sunny",           84),
    ("WEATHER_NIGHT",          "weather-night",           84),
    ("WEATHER_PARTLY_CLOUDY",  "weather-partly-cloudy",   84),
    ("WEATHER_CLOUDY",         "weather-cloudy",          84),
    ("WEATHER_POURING",        "weather-pouring",         84),
    ("WEATHER_RAINY",          "weather-rainy",           84),
    ("WEATHER_LIGHTNING",      "weather-lightning-rainy", 84),
    ("WEATHER_SNOWY",          "weather-snowy",           84),
    ("WEATHER_FOG",            "weather-fog",             84),
    # Sekcje / pasek statusu
    ("CLOCK",                  "clock-outline",           22),
    ("GIFT",                   "gift-outline",            30),
    ("THERMOMETER",            "thermometer",             18),
    ("WATER_PERCENT",          "water-percent",           18),
    ("GAUGE",                  "gauge",                   18),
    ("WEATHER_WINDY",          "weather-windy",           18),
    ("WIFI",                   "wifi",                    24),
    ("UPDATE",                 "update",                  24),
    ("CLOCK_CHECK",            "clock-check-outline",     24),
    ("SUNRISE",                "weather-sunset-up",       24),
    ("SUNSET",                 "weather-sunset-down",     24),
]

THRESHOLD = 128  # próg binaryzacji odcieni szarości -> czerń/biel


def download(url, path):
    if os.path.exists(path):
        return
    os.makedirs(os.path.dirname(path), exist_ok=True)
    print(f"Pobieram {url}")
    req = urllib.request.Request(url, headers={"User-Agent": "mdi-icon-gen"})
    with urllib.request.urlopen(req, timeout=60) as resp:
        data = resp.read()
    with open(path, "wb") as handle:
        handle.write(data)


def load_codepoints(css_path):
    """Parsuje CSS MDI: .mdi-weather-sunny::before{content:"\\F0599"} -> {'weather-sunny': 0xF0599}."""
    with open(css_path, "r", encoding="utf-8") as handle:
        css = handle.read()
    mapping = {}
    for name, hexcode in re.findall(r"\.mdi-([a-z0-9-]+)::before\{content:\"\\([0-9A-Fa-f]+)\"\}", css):
        mapping[name] = int(hexcode, 16)
    return mapping


def rasterize(font_path, codepoint, size):
    """Renderuje glif do bitmapy 1-bit; zwraca (bytes, width, height)."""
    font = ImageFont.truetype(font_path, size)
    char = chr(codepoint)
    # Renderuj na kwadratowym płótnie rozmiaru glifu; MDI mieści się w em-box.
    canvas = Image.new("L", (size, size), 0)
    from PIL import ImageDraw
    draw = ImageDraw.Draw(canvas)
    # Wyśrodkuj glif w płótnie na podstawie jego metryk.
    bbox = draw.textbbox((0, 0), char, font=font)
    glyph_w = bbox[2] - bbox[0]
    glyph_h = bbox[3] - bbox[1]
    offset_x = (size - glyph_w) // 2 - bbox[0]
    offset_y = (size - glyph_h) // 2 - bbox[1]
    draw.text((offset_x, offset_y), char, font=font, fill=255)

    width, height = size, size
    byte_width = (width + 7) // 8
    data = bytearray(byte_width * height)
    pixels = canvas.load()
    for y in range(height):
        for x in range(width):
            if pixels[x, y] >= THRESHOLD:
                data[y * byte_width + (x >> 3)] |= 0x80 >> (x & 7)
    return bytes(data), width, height


def format_array(name, data):
    lines = []
    for i in range(0, len(data), 12):
        chunk = data[i:i + 12]
        lines.append("    " + ", ".join(f"0x{b:02X}" for b in chunk) + ",")
    body = "\n".join(lines)
    return f"static const uint8_t {name}_bits[] = {{\n{body}\n}};"


def main():
    ttf_path = os.path.join(CACHE_DIR, "materialdesignicons-webfont.ttf")
    css_path = os.path.join(CACHE_DIR, "materialdesignicons.min.css")
    download(TTF_URL, ttf_path)
    download(CSS_URL, css_path)

    codepoints = load_codepoints(css_path)

    missing = [mdi for _, mdi, _ in ICONS if mdi not in codepoints]
    if missing:
        print("BŁĄD: brak w foncie MDI: " + ", ".join(missing), file=sys.stderr)
        return 1

    entries = []
    arrays = []
    for cpp_id, mdi_name, size in ICONS:
        cp = codepoints[mdi_name]
        data, width, height = rasterize(ttf_path, cp, size)
        arrays.append(f"// {mdi_name} (U+{cp:05X}), {width}x{height}")
        arrays.append(format_array(cpp_id, data))
        arrays.append("")
        entries.append((cpp_id, width, height))

    os.makedirs(os.path.dirname(OUT_PATH), exist_ok=True)
    with open(OUT_PATH, "w", encoding="utf-8", newline="\n") as out:
        out.write("// Wygenerowano przez tools/generate_mdi_icons.py — nie edytuj ręcznie.\n")
        out.write("// Ikony Material Design Icons jako bitmapy 1-bit (format Adafruit_GFX::drawBitmap).\n")
        out.write("#pragma once\n\n#include <cstdint>\n\n")
        out.write("namespace mdi {\n\n")
        out.write("\n".join(arrays))
        out.write("\nstruct Glyph { const uint8_t* bits; uint8_t width; uint8_t height; };\n\n")
        for cpp_id, width, height in entries:
            out.write(f"static const Glyph {cpp_id}{{ {cpp_id}_bits, {width}, {height} }};\n")
        out.write("\n} // namespace mdi\n")

    print(f"Zapisano {OUT_PATH} ({len(entries)} ikon)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
