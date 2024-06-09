cmake_minimum_required(VERSION 3.8)

include(git_func.cmake)

set(CMAKE_BUILD_TYPE Debug)
set(CANDLESDK_VERSION 0.1.0)
get_tag(CANDLESDK_VERSION_TAG)

if(NOT DEFINED PLATFORM)
    set(PLATFORM "native")
    message("Compiling for native system")
elseif("${PLATFORM}" STREQUAL "WIN")
    message("Cross - compiling for Windows")
    set(CMAKE_CXX_COMPILER /usr/bin/x86_64-w64-mingw32-g++-posix)
    set(CMAKE_C_COMPILER /usr/bin/x86_64-w64-mingw32-gcc-posix)
    set(CMAKE_SYSTEM_NAME Windows)
endif()
project(candlesdk)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
    -Wfatal-errors
    -Wall
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
    -Wno-missing-field-initializers
    -Werror
)

set(mINI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/mINI/inc/)

set(CANDLE_BUILD_STATIC TRUE)
add_subdirectory(commons)
add_subdirectory(candlelib)
add_subdirectory(mdtool)
# add_subdirectory(examples)