# ImGui library
file(GLOB IMGUI_SOURCES
    "imgui.cpp"
    "imgui_demo.cpp"
    "imgui_draw.cpp"
    "imgui_tables.cpp"
    "imgui_widgets.cpp"
    "backends/imgui_impl_dx9.cpp"
    "backends/imgui_impl_win32.cpp"
    "misc/freetype/imgui_freetype.cpp"
)

file(GLOB IMGUI_HEADERS
    "imgui.h"
    "imconfig.h"
    "imgui_internal.h"
    "imstb_rectpack.h"
    "imstb_textedit.h"
    "imstb_truetype.h"
    "backends/imgui_impl_dx9.h"
    "backends/imgui_impl_win32.h"
    "misc/freetype/imgui_freetype.h"
)

add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS})

target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/misc/freetype
    ${CMAKE_SOURCE_DIR}/vendor/freetype/include
)

target_link_libraries(imgui PUBLIC
    freetype
    d3d9
)

target_compile_definitions(imgui PUBLIC
    IMGUI_ENABLE_FREETYPE
    IMGUI_USE_FREETYPE
)

GroupSourcesByFolder(imgui)
