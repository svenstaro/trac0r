cmake_minimum_required(VERSION 3.0)
project(trac0r)
include(ExternalProject)

add_definitions("-O2 -g -Wall -Wextra -pedantic -Werror -std=c++14 -Wno-unused-parameter")
#set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -flto")

set(cppformat external/cppformat/format.cc)
file(GLOB trac0r_library_src trac0r/*.cpp ${cppformat})
file(GLOB trac0r_tests_src tests/*.cpp)
file(GLOB trac0r_viewer_src viewer/*.cpp)

add_library(trac0r_library ${trac0r_library_src})
add_executable(trac0r_viewer ${trac0r_viewer_src})
add_executable(trac0r_tests ${trac0r_tests_src})

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
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(trac0r_viewer
    trac0r_library
    ${SDL2_LIBRARIES}
)

target_link_libraries(trac0r_tests
    trac0r_library
    ${SDL2_LIBRARIES}
)
