cmake_minimum_required(VERSION 3.0)
project(trac0r)
include(ExternalProject)

add_definitions("-g -O3 -Wall -Wextra -pedantic -Werror -std=c++14 -Wno-unused-parameter")

file(GLOB trac0r_sources trac0r/*.cpp)
file(GLOB trac0r_tests tests/*.cpp)
file(GLOB trac0r_viewer viewer/*.cpp)

add_library(trac0r ${trac0r_sources})
add_executable(trac0r_viewer ${trac0r_viewer})
add_executable(trac0r_tests ${trac0r_tests})

if(${EMSCRIPTEN})
    set(CROSS_COMPILING ON)
    add_definitions("-Oz -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2")
    set(CMAKE_CXX_LINK_FLAGS "-O3 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s TOTAL_MEMORY=32000000 --preload-file ../res -o index.html")
else()
    include(FindPkgConfig)
    pkg_check_modules(SDL2 REQUIRED sdl2 SDL2_image SDL2_ttf)
endif()

ExternalProject_Add(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/glm
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(glm source_dir)
set(glm_INCLUDE_DIR ${source_dir})

ExternalProject_Add(cppformat
    GIT_REPOSITORY https://github.com/cppformat/cppformat.git
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/cppformat
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(cppformat binary_dir)
set(cppformat_LIBRARIES ${binary_dir}/libcppformat.a)

add_dependencies(trac0r glm cppformat)

include_directories(SYSTEM
    ${SDL2_INCLUDE_DIRS}
    ${glm_INCLUDE_DIR}
    external/
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(trac0r_viewer
    trac0r
    ${cppformat_LIBRARIES}
    ${SDL2_LIBRARIES}
)

target_link_libraries(trac0r_tests
    trac0r
    ${cppformat_LIBRARIES}
    ${SDL2_LIBRARIES}
)
