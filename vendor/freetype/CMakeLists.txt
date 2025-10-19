# FreeType2 library - building from source

# Disable unnecessary FreeType features
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "Disable HarfBuzz" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "Disable Brotli" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "Disable BZip2" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "Disable PNG" FORCE)
set(FT_DISABLE_ZLIB OFF CACHE BOOL "Enable ZLIB" FORCE)

# Build FreeType from source
add_subdirectory(freetype-src)
