# FreeTypeFont.txt - Configuration Guide

## Overview

The `FreeTypeFont.txt` file is used to configure fonts loaded by the ImGui + FreeType rendering system in the Metin2 client. This file allows you to define multiple fonts with different sizes, quality settings, and outline effects.

## File Location

Place this file in the game's root directory:
```
Metin2_RelWithDebInfo.exe
FreeTypeFont.txt          <-- Here
font/
  Arial.ttf
  Tahoma.ttf
  ...
```

## Configuration Syntax

Each font is defined by a series of key-value pairs. Comments start with `//` or `#`.

### Basic Structure

```
# Font definition starts with FONT directive
FONT                 <font_name>
PATH                 <path_to_ttf_file>
SIZE                 <font_size_in_pixels>
OVERSAMPLE_H         <horizontal_oversampling>
OVERSAMPLE_V         <vertical_oversampling>
RASTERIZER_MULTIPLY  <brightness_multiplier>
ENABLE_OUTLINE       <0_or_1>
OUTLINE_THICKNESS    <thickness_in_pixels>
PIXEL_SNAP_H         <0_or_1>

# Next font definition...
FONT                 <another_font_name>
...
```

---

## Configuration Parameters

### **FONT** (Required)
- **Type:** String
- **Description:** Unique identifier for this font. Used in code to reference the font.
- **Example:** `UI_DEF_FONT`, `UI_DEF_FONT_LARGE`, `UI_DEF_FONT_SMALL`
- **Notes:** Use descriptive names. This is how the font is accessed in code.

---

### **PATH** (Required)
- **Type:** File path (relative or absolute)
- **Description:** Path to the TrueType Font (.ttf) file.
- **Example:** `font\Arial.ttf`, `font\Tahoma.ttf`, `C:\Windows\Fonts\Arial.ttf`
- **Notes:** Use backslashes (`\`) for Windows paths. Forward slashes (`/`) also work.

---

### **SIZE** (Required)
- **Type:** Float
- **Description:** Font size in pixels.
- **Example:** `14.0`, `16.0`, `20.0`
---

### **OVERSAMPLE_H** (Optional)
- **Type:** Integer (1-8)
- **Description:** Horizontal oversampling for smoother font rendering.
- **Default:** `4`
- **Recommended:** `4` (best balance of quality and performance)
- **Notes:** Higher values = smoother but slower. Values above 4 rarely improve quality.

---

### **OVERSAMPLE_V** (Optional)
- **Type:** Integer (1-8)
- **Description:** Vertical oversampling for smoother font rendering.
- **Default:** `4`
- **Recommended:** `4` (best balance of quality and performance)
- **Notes:** Higher values = smoother but slower. Values above 4 rarely improve quality.

---

### **RASTERIZER_MULTIPLY** (Optional)
- **Type:** Float (0.0 - 2.0)
- **Description:** Brightness multiplier applied during font rasterization.
- **Default:** `1.2`
- **Recommended:**
  - Light fonts: `1.0` - `1.2`
  - Normal fonts: `1.2` - `1.3`
  - Bold fonts: `1.3` - `1.5`
- **Notes:** Higher values make the font appear bolder/brighter. Too high can cause artifacts.

---

### **ENABLE_OUTLINE** (Optional)
- **Type:** Boolean (0 or 1)
- **Description:** Enable text outline rendering.
- **Default:** `0` (disabled)
- **Values:**
  - `0` = No outline
  - `1` = Outline enabled (uses OUTLINE_THICKNESS)
- **Notes:** Outline improves text readability on complex backgrounds. Costs 8 additional draw calls per text.

---

### **OUTLINE_THICKNESS** (Optional)
- **Type:** Integer (0-3)
- **Description:** Thickness of the text outline in pixels.
- **Default:** `1`
- **Recommended:**
  - Small fonts (12-14px): `1`
  - Medium fonts (16-18px): `1` or `2`
  - Large fonts (20+px): `2` or `3`
- **Notes:**
  - Only used if `ENABLE_OUTLINE 1`
  - Values > 3 may look too thick
  - Creates 8-directional outline (N, NE, E, SE, S, SW, W, NW)

---

### **PIXEL_SNAP_H** (Optional)
- **Type:** Boolean (0 or 1)
- **Description:** Snap glyph positions to pixel grid horizontally.
- **Default:** `0` (disabled)
- **Recommended:**
  - Small fonts: `1` (crisper)
  - Large/smooth fonts: `0` (smoother)
- **Notes:** Can make small fonts appear sharper but may cause spacing issues.

---

## Complete Example Configuration

```ini
# ===================================================================
# METIN2 CLIENT - FREETYPE FONT CONFIGURATION
# ===================================================================
# This file configures all fonts used in the game UI.
# Modify carefully - incorrect values may cause rendering issues.
# ===================================================================

# ===================================================================
# DEFAULT UI FONT (UI_DEF_FONT)
# ===================================================================
FONT                 UI_DEF_FONT
PATH                 font\Arial.ttf
SIZE                 12.0
OVERSAMPLE_H         4
OVERSAMPLE_V         4
RASTERIZER_MULTIPLY  1.3
ENABLE_OUTLINE       1
OUTLINE_THICKNESS    1
PIXEL_SNAP_H         1

# ===================================================================
# LARGE UI FONT (UI_DEF_FONT_LARGE)
# ===================================================================
FONT                 UI_DEF_FONT_LARGE
PATH                 font\Arial.ttf
SIZE                 14.0
OVERSAMPLE_H         4
OVERSAMPLE_V         4
RASTERIZER_MULTIPLY  1.2
ENABLE_OUTLINE       1
OUTLINE_THICKNESS    2
PIXEL_SNAP_H         0

# ===================================================================
# CHAT FONT (UI_DEF_FONT_SMALL)
# ===================================================================
FONT                 UI_DEF_FONT_SMALL
PATH                 font\Arial.ttf
SIZE                 9.0
OVERSAMPLE_H         4
OVERSAMPLE_V         4
RASTERIZER_MULTIPLY  1.4
ENABLE_OUTLINE       1
OUTLINE_THICKNESS    1
PIXEL_SNAP_H         1


# ===================================================================
# CAN BE IMPROVED / DVELOPED BY
# ===================================================================


# ===================================================================
# DAMAGE NUMBERS (DMG_FONT)
# Used for: Floating damage numbers
# ===================================================================
FONT                 DMG_FONT
PATH                 font\ArialBold.ttf
SIZE                 24.0
OVERSAMPLE_H         4
OVERSAMPLE_V         4
RASTERIZER_MULTIPLY  1.5
ENABLE_OUTLINE       1
OUTLINE_THICKNESS    2
PIXEL_SNAP_H         0

# ===================================================================
# TOOLTIP FONT (TOOLTIP_FONT)
# Used for: Item tooltips, hover text
# ===================================================================
FONT                 TOOLTIP_FONT
PATH                 font\Arial.ttf
SIZE                 13.0
OVERSAMPLE_H         4
OVERSAMPLE_V         4
RASTERIZER_MULTIPLY  1.2
ENABLE_OUTLINE       0
OUTLINE_THICKNESS    0
PIXEL_SNAP_H         1
```

---

## Advanced Topics

### FreeType Quality Settings

The game uses **FreeType rasterizer** which provides superior font quality compared to the default stb_truetype. The following quality preset is used globally:

```cpp
// In code (ImGuiManager.cpp)
EFreeTypeQuality::HighQuality = NoHinting | LightHinting
```

This provides the best balance of sharpness and smoothness for most fonts.

### Supported Character Ranges

The following Unicode ranges are loaded automatically:

- **Basic Latin + Latin Supplement:** `0x0020 - 0x00FF`
- **Latin Extended-A:** `0x0100 - 0x017F`
- **Latin Extended-B:** `0x0180 - 0x024F`
- **Cyrillic + Cyrillic Supplement:** `0x0400 - 0x052F`
- **General Punctuation:** `0x2000 - 0x206F`
- **CJK Symbols and Punctuation, Hiragana, Katakana:** `0x3000 - 0x30FF`
- **CJK Ideograms:** `0x4E00 - 0x9FAF`
- **Hangul Syllables:** `0xAC00 - 0xD7A3`

### Outline Rendering Details

When `ENABLE_OUTLINE 1`:

1. The font is rendered 8 times with offset positions:
   ```
   N (0, -thickness)    NE (+thickness, -thickness)    E (+thickness, 0)
   SE (+thickness, +thickness)    S (0, +thickness)    SW (-thickness, +thickness)
   W (-thickness, 0)    NW (-thickness, -thickness)
   ```

2. The main text is rendered centered (0, 0) on top.

3. This creates a proper **outline** effect (text with border) NOT a shadow effect.

### Performance Considerations

- **OVERSAMPLE_H/V:** Values of 4 are optimal. Higher values have diminishing returns.
- **OUTLINE:** Adds 8 draw calls per text. Use sparingly for critical text only.
- **RASTERIZER_MULTIPLY:** No performance impact, only affects brightness.
- **Font Atlas Size:** More fonts = larger texture atlas. Typically not an issue unless you load 50+ fonts.

---

## Troubleshooting

### Problem: Font doesn't load
**Solution:** Check that `PATH` is correct and the .ttf file exists.

### Problem: Outline looks like a shadow (shifted right)
**Solution:** Make sure you're using the latest version of ImGuiManager.cpp with centered outline rendering.

### Problem: Text looks blurry
**Solution:**
- Increase `RASTERIZER_MULTIPLY` to `1.3` - `1.5`
- Ensure `OVERSAMPLE_H` and `OVERSAMPLE_V` are set to `4`

### Problem: Small text looks jaggy
**Solution:**
- Enable `PIXEL_SNAP_H 1`
- Reduce `SIZE` slightly and increase `RASTERIZER_MULTIPLY`

### Problem: Outline is too thick/thin
**Solution:** Adjust `OUTLINE_THICKNESS` (typically 1-2 is best).

---

## Notes

- Comments can start with `//` or `#`
- Blank lines are ignored
- Parameter order doesn't matter (but keep them grouped for readability)
- Font names are case-sensitive
- If a font fails to load, the game will fall back to Arial.ttf or Tahoma.ttf
- Changes to this file require a game restart to take effect

