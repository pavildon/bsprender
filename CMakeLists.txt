cmake_minimum_required(VERSION 3.1)
project(tessellation)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 ")

#IF(APPLE)
    include_directories(/System/Library/Frameworks)
    include_directories(/usr/local/include)
    find_library(OPENGL OpenGL)
    find_library(COCOA Cocoa)
    find_library(IOKIT IOKit)
    find_library(COREVIDEO CoreVideo)
    mark_as_advanced(
        OPENGL
        COCOA
        IOKIT
        COREVIDEO
    )
    set(EXTRA_LIBS ${OPENGL} ${IOKIT} ${COCOA} ${COREVIDEO})
#ENDIF(APPLE)

include(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
#PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image)o

include_directories(${SDL2_INCLUDE_DIR})
link_directories(/usr/local/lib)

#find_package(glfw3 REQUIRED)
#find_package(png REQUIRED)

#include_directories(${PNG_INCLUDE_DIR})
#include_directories(${GLFW3_INCLUD_DIR})
#link_directories(${GLFW3_LIBRARY_DIR})

add_library(soil STATIC soil/image_DXT.c soil/image_DXT.h soil/SOIL.c
        soil/SOIL.h soil/stb_image_aug.c soil/stb_image_aug.h soil/stbi_DDS_aug.h soil/stb_image_aug.h
        soil/stbi_DDS_aug_c.h soil/image_helper.c soil/image_helper.h)

set(SOURCE_FILES main.cpp Benchmarks.cpp Benchmarks.h tga/tga.h tga/tga.cpp tga/tga.h)

add_executable(tessellation ${SOURCE_FILES} shaders.h simple_cube.h input.h input.cpp common.h bsp/bsp.h bsp/bsp.cpp shader_sources.h bsp/Bezier.cpp bsp/Bezier.h ${SOIL_LIB})
target_link_libraries(tessellation soil ${EXTRA_LIBS} ${SDL2_LIBRARIES}  )

