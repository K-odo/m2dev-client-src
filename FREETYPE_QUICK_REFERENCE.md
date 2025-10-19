# FreeTypeFont.txt - Quick Reference

## Minimal Configuration

```ini
FONT                 UI_DEF_FONT
PATH                 font\Arial.ttf
SIZE                 14.0
OVERSAMPLE_H         4
OVERSAMPLE_V         4
RASTERIZER_MULTIPLY  1.3
ENABLE_OUTLINE       1
OUTLINE_THICKNESS    1
PIXEL_SNAP_H         1
```

---

## Parameter Quick Reference

| Parameter | Type | Range | Default | Description |
|-----------|------|-------|---------|-------------|
| **FONT** | String | - | Required | Unique font identifier |
| **PATH** | Path | - | Required | Path to .ttf file |
| **SIZE** | Float | 8.0-72.0 | Required | Font size in pixels |
| **OVERSAMPLE_H** | Int | 1-8 | 4 | Horizontal oversampling |
| **OVERSAMPLE_V** | Int | 1-8 | 4 | Vertical oversampling |
| **RASTERIZER_MULTIPLY** | Float | 0.5-2.0 | 1.2 | Brightness multiplier |
| **ENABLE_OUTLINE** | Bool | 0-1 | 0 | Enable text outline |
| **OUTLINE_THICKNESS** | Int | 0-3 | 1 | Outline thickness (pixels) |
| **PIXEL_SNAP_H** | Bool | 0-1 | 0 | Snap to pixel grid |

---

## Recommended Presets

### **Small UI Text (12px)**
```ini
SIZE                 12.0
RASTERIZER_MULTIPLY  1.4
OUTLINE_THICKNESS    1
PIXEL_SNAP_H         1
```

### **Normal UI Text (14px)**
```ini
SIZE                 14.0
RASTERIZER_MULTIPLY  1.3
OUTLINE_THICKNESS    1
PIXEL_SNAP_H         1
```

### **Large Headers (18-20px)**
```ini
SIZE                 18.0
RASTERIZER_MULTIPLY  1.2
OUTLINE_THICKNESS    2
PIXEL_SNAP_H         0
```

### **Damage Numbers (bold, visible)**
```ini
SIZE                 16.0
RASTERIZER_MULTIPLY  1.5
OUTLINE_THICKNESS    2
PIXEL_SNAP_H         0
```

### **Chat Text (compact, readable)**
```ini
SIZE                 12.0
RASTERIZER_MULTIPLY  1.4
OUTLINE_THICKNESS    1
PIXEL_SNAP_H         1
```

### **Tooltips (clean, no outline)**
```ini
SIZE                 13.0
RASTERIZER_MULTIPLY  1.2
ENABLE_OUTLINE       0
PIXEL_SNAP_H         1
```

---

## Common Adjustments

### **Make text brighter/bolder**
```ini
RASTERIZER_MULTIPLY  1.5    # Increase (was 1.2)
```

### **Make text sharper**
```ini
PIXEL_SNAP_H         1      # Enable pixel snapping
```

### **Make outline thicker**
```ini
OUTLINE_THICKNESS    2      # Increase (was 1)
```

### **Disable outline completely**
```ini
ENABLE_OUTLINE       0      # Turn off
OUTLINE_THICKNESS    0      # Set to 0
```

### **Make text smoother**
```ini
PIXEL_SNAP_H         0      # Disable pixel snapping
```

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Text is blurry | Increase `RASTERIZER_MULTIPLY` to 1.4-1.5 |
| Text is too thin | Increase `RASTERIZER_MULTIPLY` |
| Small text is jaggy | Enable `PIXEL_SNAP_H 1` |
| Outline looks like shadow | **Fixed in latest code!** Outline is now centered |
| Outline too thick | Reduce `OUTLINE_THICKNESS` to 1 |
| Font doesn't load | Check `PATH` is correct |
| Poor performance | Reduce number of fonts, or disable outline on some |

---

## Supported Fonts

Any TrueType Font (.ttf) file works:
- Arial.ttf ✅
- Tahoma.ttf ✅
- Times New Roman.ttf ✅
- Custom TTF files ✅

**Supported languages:** Latin, Cyrillic, CJK (Chinese, Japanese, Korean), Hangul

---

## Performance Tips

1. **Use OVERSAMPLE 4:** Higher values (5-8) rarely improve quality
2. **Limit outline usage:** Outline adds 8 draw calls per text
3. **Reuse fonts:** Define shared fonts once, use everywhere
4. **Don't over-configure:** 5-8 fonts is usually enough

---
```

## Example: Complete 3-Font Setup


# Main FONT
FONT                 UI_DEF_FONT
PATH                 font\Arial.ttf
SIZE                 12.0
OVERSAMPLE_H         4
OVERSAMPLE_V         4
RASTERIZER_MULTIPLY  1.3
ENABLE_OUTLINE       1
OUTLINE_THICKNESS    1
PIXEL_SNAP_H         1

# Large FONT
FONT                 UI_DEF_FONT_LARGE
PATH                 font\Arial.ttf
SIZE                 14.0
OVERSAMPLE_H         4
OVERSAMPLE_V         4
RASTERIZER_MULTIPLY  1.2
ENABLE_OUTLINE       1
OUTLINE_THICKNESS    2
PIXEL_SNAP_H         0

# SMALL FONT
FONT                 UI_DEF_FONT_SMALL
PATH                 font\Tahoma.ttf
SIZE                 9.0
OVERSAMPLE_H         4
OVERSAMPLE_V         4
RASTERIZER_MULTIPLY  1.4
ENABLE_OUTLINE       1
OUTLINE_THICKNESS    1
PIXEL_SNAP_H         1

```

---

**For full documentation, see:** `FREETYPE_FONT_CONFIG.md`
