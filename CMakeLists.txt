cmake_minimum_required(VERSION 3.0)
project(trac0r)

add_definitions("-g -O3 -Wall -Wextra -pedantic -Werror -std=c++14 -Wno-unused-parameter")

file(GLOB trac0r_sources trac0r/*.cpp)
file(GLOB trac0r_viewer viewer/*.cpp)

add_library(trac0r ${trac0r_sources})
add_executable(trac0r_viewer ${trac0r_viewer})

if(${EMSCRIPTEN})
    set(CROSS_COMPILING ON)
    add_definitions("-Oz -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2")
    set(CMAKE_CXX_LINK_FLAGS "-O3 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s TOTAL_MEMORY=32000000 --preload-file ../res -o index.html")
else()
    include(FindPkgConfig)
    pkg_check_modules(SDL2 REQUIRED sdl2 SDL2_image SDL2_ttf)
endif()

include_directories(SYSTEM
    ${SDL2_INCLUDE_DIRS}
    glm/
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(trac0r_viewer
    trac0r
    ${SDL2_LIBRARIES}
)
