This index helps you find the right documentation for your needs.

---
## All Documentation Files

### **Font Configuration**

| File | Purpose | When to Use |
|------|---------|-------------|
| **FREETYPE_QUICK_REFERENCE.md** | Fast lookup for common font settings | Quick adjustments, troubleshooting |
| **FREETYPE_FONT_CONFIG.md** | Complete parameter reference | Deep dive into all options |
| **FreeTypeFont.txt.example** | Ready-to-use config template | Copy as starting point |
| **FREETYPE_FONTS_GUIDE.md** | Legacy guide (older version) | Reference for migration |

**Start here:** `FREETYPE_QUICK_REFERENCE.md` → `FreeTypeFont.txt.example`

---


### **I want to fix blurry/jaggy text**

**Blurry text:**
- Increase `RASTERIZER_MULTIPLY` from `1.2` to `1.4` or `1.5`
- Ensure `OVERSAMPLE_H 4` and `OVERSAMPLE_V 4`

**Jaggy small text:**
- Enable `PIXEL_SNAP_H 1`
- Ensure size is not too small (minimum 11-12px recommended)

**Outline looks wrong:**
- Make sure you have the **latest code** (outline is now centered, not shifted)
- Adjust `OUTLINE_THICKNESS` (1-2 is usually best)

See: `FREETYPE_QUICK_REFERENCE.md` → Troubleshooting section

---

## Technical Reference

### **Font System Architecture**

```
FreeTypeFont.txt (config)
    ↓ (parsed by)
CImGuiManager::LoadFontsFromConfig()
    ↓ (creates)
ImFont* objects (FreeType rasterized)
    ↓ (stored in)
m_mapFonts (font registry)
    ↓ (rendered by)
CImGuiManager::RenderText() / RenderTextWithOutline()
    ↓ (using)
ImGui DrawLists → DirectX9
```

### **Outline Rendering Algorithm**

```
When ENABLE_OUTLINE 1:
  For each of 8 directions (N, NE, E, SE, S, SW, W, NW):
    DrawText at (x + offset_x * thickness, y + offset_y * thickness) in outline color
  DrawText at (x, y) in main text color (centered on top)

Result: Proper outline around text (NOT shadow effect)
```

### **Key Code Locations**

| What | Where |
|------|-------|
| Font loading | `src/EterLib/ImGuiManager.cpp:230-297` |
| Outline rendering | `src/EterLib/ImGuiManager.cpp:559-606` |
| Config parsing | `src/EterLib/ImGuiManager.cpp:96-228` |
| Integration | `src/UserInterface/PythonApplication.cpp:1065, 587-599` |
| Font structures | `src/EterLib/ImGuiManager.h:34-68` |

---

## Learning Path

### **Beginner** (30 min)
1. `FREETYPE_QUICK_REFERENCE.md` - Configure fonts
2. `FreeTypeFont.txt.example` - Copy and use

### **Intermediate** (60 min)
4. `FREETYPE_FONT_CONFIG.md` - Understand all parameters
5. Edit fonts in `FreeTypeFont.txt` and experiment

---

## Help & Support

### **Common Issues**

| Issue | Solution File |
|-------|---------------|
| Font doesn't load | `FREETYPE_FONT_CONFIG.md` → Troubleshooting |
| Outline looks wrong | `FREETYPE_QUICK_REFERENCE.md` → Troubleshooting |
| Performance issues | `FREETYPE_FONT_CONFIG.md` → Performance section |
| Text quality issues | `FREETYPE_QUICK_REFERENCE.md` → Common Adjustments |

