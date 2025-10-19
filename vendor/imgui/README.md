# ImGui Integration for Metin2 Client

## Quick Setup (Windows x64)

### 1. Download ImGui

```bash
cd vendor/imgui
git clone https://github.com/ocornut/imgui.git .
```

**OR** manually download from: https://github.com/ocornut/imgui/releases

### 2. Download FreeType2

**Option A: Prebuilt (RECOMMENDED)**
- Download from: https://github.com/ubawurinna/freetype-windows-binaries/releases
- Extract to `vendor/freetype/`
- Ensure structure:
  ```
  vendor/freetype/
    include/
      ft2build.h
      freetype/...
    lib/
      freetype.lib
  ```

**Option B: Build from source**
```bash
git clone https://github.com/freetype/freetype.git vendor/freetype/freetype-src
```
Then modify `vendor/freetype/CMakeLists.txt` accordingly.

### 3. Build

```bash
mkdir build && cd build
cmake ..
cmake --build . --config RelWithDebInfo
```

---

## Required ImGui Files

Core files needed from ImGui repository:

### Base
- `imgui.cpp`, `imgui.h`
- `imgui_draw.cpp`
- `imgui_tables.cpp`
- `imgui_widgets.cpp`
- `imgui_demo.cpp` (optional)
- `imgui_internal.h`
- `imconfig.h`
- `imstb_rectpack.h`, `imstb_textedit.h`, `imstb_truetype.h`

### Backends
- `backends/imgui_impl_dx9.cpp`
- `backends/imgui_impl_dx9.h`
- `backends/imgui_impl_win32.cpp`
- `backends/imgui_impl_win32.h`

### FreeType Extension
- `misc/freetype/imgui_freetype.cpp`
- `misc/freetype/imgui_freetype.h`

---

## Features

| Feature | Status |
|---------|--------|
| DirectX9 backend | ✅ Enabled |
| Win32 platform | ✅ Enabled |
| FreeType rasterization | ✅ Enabled |
| Full antialiasing | ✅ Enabled |
| Kerning | ✅ Enabled |
| Outline rendering | ✅ Enabled |
| Unicode (CJK/Cyrillic) | ✅ Enabled |
| High DPI support | ✅ Enabled |

---

## Verification

After downloading all files, verify your structure:

```
vendor/imgui/
├── imgui.cpp
├── imgui.h
├── imgui_draw.cpp
├── imgui_tables.cpp
├── imgui_widgets.cpp
├── imgui_internal.h
├── imconfig.h
├── imstb_*.h
├── backends/
│   ├── imgui_impl_dx9.cpp
│   ├── imgui_impl_dx9.h
│   ├── imgui_impl_win32.cpp
│   └── imgui_impl_win32.h
├── misc/freetype/
│   ├── imgui_freetype.cpp
│   └── imgui_freetype.h
├── CMakeLists.txt
└── README.md
```

---

## Troubleshooting

**Build error: "Cannot find imgui.h"**
- Ensure all ImGui files are in `vendor/imgui/`
- Run `git clone` or download manually

**Build error: "Cannot find freetype.lib"**
- Download FreeType2 prebuilt binaries
- Place in `vendor/freetype/lib/freetype.lib`

**Runtime error: "Failed to load font"**
- Check font path (e.g., `C:\\Windows\\Fonts\\arial.ttf`)
- Ensure FreeType is properly linked

---

For complete integration guide, see: **IMGUI_INTEGRATION_GUIDE.md** in project root.
