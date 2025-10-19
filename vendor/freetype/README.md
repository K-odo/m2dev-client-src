# FreeType2 for Metin2 Client

## Quick Setup (Windows x64)

### Option 1: Prebuilt Binaries (RECOMMENDED)

1. **Download prebuilt FreeType2**
   - URL: https://github.com/ubawurinna/freetype-windows-binaries/releases
   - Download latest x64 release (e.g., `freetype-windows-binaries-2.13.2.zip`)

2. **Extract to this directory**
   ```
   vendor/freetype/
   ├── include/
   │   ├── ft2build.h
   │   └── freetype/
   │       ├── freetype.h
   │       ├── ftadvanc.h
   │       ├── ftbitmap.h
   │       └── ... (all FreeType headers)
   ├── lib/
   │   └── freetype.lib  (x64 static library)
   ├── CMakeLists.txt
   └── README.md (this file)
   ```

3. **Verify the structure**
   - Check that `lib/freetype.lib` exists
   - Check that `include/ft2build.h` exists

---

### Option 2: Build from Source

1. **Clone FreeType repository**
   ```bash
   cd vendor/freetype
   git clone https://github.com/freetype/freetype.git freetype-src
   cd freetype-src
   git checkout VER-2-13-2  # or latest stable tag
   ```

2. **Modify CMakeLists.txt**

   Edit `vendor/freetype/CMakeLists.txt`:

   ```cmake
   # Comment out the IMPORTED library
   # add_library(freetype STATIC IMPORTED GLOBAL)
   # set_target_properties(freetype PROPERTIES ...)

   # Add this instead:
   set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "Disable HarfBuzz" FORCE)
   set(FT_DISABLE_BROTLI ON CACHE BOOL "Disable Brotli" FORCE)
   set(FT_DISABLE_BZIP2 ON CACHE BOOL "Disable BZip2" FORCE)
   set(FT_DISABLE_PNG ON CACHE BOOL "Disable PNG" FORCE)
   set(FT_DISABLE_ZLIB ON CACHE BOOL "Disable ZLIB" FORCE)

   add_subdirectory(freetype-src)
   ```

3. **Build**
   ```bash
   cd ../../../  # Back to project root
   mkdir build && cd build
   cmake ..
   cmake --build . --config Release
   ```

---

## Current Configuration

This directory is configured to use **prebuilt static library** via CMake's IMPORTED target.

The `CMakeLists.txt` expects:
- **Headers**: `include/ft2build.h` and `include/freetype/*.h`
- **Library**: `lib/freetype.lib` (64-bit static)

---

## Verification Checklist

After setup, verify:

- [ ] `vendor/freetype/include/ft2build.h` exists
- [ ] `vendor/freetype/include/freetype/freetype.h` exists
- [ ] `vendor/freetype/lib/freetype.lib` exists (x64)
- [ ] CMake configuration succeeds
- [ ] Project builds without linker errors

---

## Troubleshooting

### Build Error: "Cannot find ft2build.h"

**Solution:**
- Ensure `include/ft2build.h` exists in this directory
- Check `CMakeLists.txt` has correct `INTERFACE_INCLUDE_DIRECTORIES`

### Linker Error: "Cannot find freetype.lib"

**Solution:**
- Download prebuilt binaries (see Option 1)
- Ensure `lib/freetype.lib` is x64 architecture
- Check `CMakeLists.txt` has correct `IMPORTED_LOCATION`

### Runtime Error: "Font rendering broken"

**Solution:**
- Verify FreeType version ≥ 2.10
- Ensure ImGui's `IMGUI_ENABLE_FREETYPE` is defined
- Check that `ImGuiFreeType::BuildFontAtlas()` is called

---

## Version Compatibility

| FreeType Version | Status |
|------------------|--------|
| 2.13.x | ✅ Recommended |
| 2.12.x | ✅ Compatible |
| 2.11.x | ✅ Compatible |
| 2.10.x | ⚠️ Minimum |
| < 2.10 | ❌ Not supported |

---

## Additional Resources

- **FreeType Official**: https://freetype.org/
- **FreeType Docs**: https://freetype.org/freetype2/docs/documentation.html
- **ImGui FreeType**: https://github.com/ocornut/imgui/blob/master/misc/freetype/README.md
- **Prebuilt Binaries**: https://github.com/ubawurinna/freetype-windows-binaries

---

## License

FreeType is licensed under the **FreeType License** (BSD-style).
See: https://freetype.org/license.html
