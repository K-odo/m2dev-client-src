file(GLOB_RECURSE FILE_SOURCES "*.h" "*.c" "*.cpp")

add_library(EterLib STATIC ${FILE_SOURCES})

target_link_libraries(EterLib
	cryptopp-static
	mio
	imgui
	freetype
)

target_include_directories(EterLib PUBLIC
	${CMAKE_SOURCE_DIR}/vendor/imgui
	${CMAKE_SOURCE_DIR}/vendor/freetype/include
)

GroupSourcesByFolder(EterLib)
