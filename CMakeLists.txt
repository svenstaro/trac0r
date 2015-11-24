cmake_minimum_required(VERSION 3.0)
project(trac0r)
include(ExternalProject)

set(trac0r_flags -O2 -g -Wall -Wextra -pedantic -Werror -std=c++14 -Wno-unused-parameter)
#set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -flto")

file(GLOB cppformat_src external/cppformat/*.cc)
add_library(cppformat ${cppformat_src})

file(GLOB sdl2_gfx_src external/sdl2_gfx/*.c)
add_library(sdl2_gfx ${sdl2_gfx_src})

file(GLOB trac0r_library_src trac0r/*.cpp)
file(GLOB trac0r_viewer_src viewer/*.cpp)

add_library(trac0r_library ${trac0r_library_src})
add_executable(trac0r_viewer ${trac0r_viewer_src})
add_executable(trac0r_test_camera tests/test_camera.cpp)
add_executable(trac0r_test_packing tests/test_packing.cpp)

target_compile_options(trac0r_library PUBLIC ${trac0r_flags})
target_compile_options(trac0r_viewer PUBLIC ${trac0r_flags})
target_compile_options(trac0r_test_camera PUBLIC ${trac0r_flags})
target_compile_options(trac0r_test_packing PUBLIC ${trac0r_flags})

if(${BENCHMARK})
    add_definitions("-DBENCHMARK")
endif()

if(${EMSCRIPTEN})
    set(CROSS_COMPILING ON)
    add_definitions("-O3 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2")
    set(CMAKE_CXX_LINK_FLAGS "-O3 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s TOTAL_MEMORY=32000000 --emrun --preload-file ../res -o index.html")
else()
    include(FindPkgConfig)
    pkg_check_modules(SDL2 REQUIRED sdl2 SDL2_image SDL2_ttf)
endif()

include_directories(SYSTEM
    ${SDL2_INCLUDE_DIRS}
    external/
    external/glm
    external/sdl2_gfx
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(trac0r_library cppformat)

target_link_libraries(trac0r_viewer
    trac0r_library
    sdl2_gfx
    ${SDL2_LIBRARIES}
)

target_link_libraries(trac0r_test_camera trac0r_library)
target_link_libraries(trac0r_test_packing trac0r_library)
