cmake_minimum_required(VERSION 3.14)

set(CMAKE_BUILD_TYPE "Debug")

project(video-app C CXX)

set(CMAKE_CXX_DEBUG_FLAGS "${CMAKE_CXX_DEBUG_FLAGS} -g -glldb")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
message("CMAKE_CXX_DEBUG_FLAGS: ${CMAKE_CXX_DEBUG_FLAGS}")

add_subdirectory(lib/glfw)
add_subdirectory(lib/FFmpeg)
add_subdirectory(lib/utils)
add_definitions(-DGL_SILENCE_DEPRECATION)


list(APPEND SOURCES
	src/main.cpp
	src/video_reader.cpp
	src/video_reader.hpp
)
if(APPLE)
list(APPEND EXTRA_LIBS
"-framework OpenGL"
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/assets/MacOSXBundleInfo.plist.in
	${CMAKE_CURRENT_BINARY_DIR}/assets/MacOSXBundleInfo.plist
    )

    set_target_properties(video-app PROPERTIES
	MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/assets/MacOSXBundleInfo.plist
    )

	elseif(WIN32)
    list(APPEND EXTRA_LIBS
	"-lglu32 -lopengl32"
    )
    set(CMAKE_EXE_LINKER_FLAGS "-std=gnu99 -static -static-libgcc -static-libstdc++ -mwindows")
	else()
    list(APPEND EXTRA_LIBS
	"-lGL -lGLU -lX11"
    )
	endif()

add_executable(video-app ${SOURCES})
target_link_libraries(video-app FFmpeg glfw ${EXTRA_LIBS} avutil utils GLEW)


