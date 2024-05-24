cmake_minimum_required(VERSION 3.5)

project(candlelib)

# change the version and tag here
set(CANDLE_LIB_VERSION 3.5.0)
set(VERSION_TAG 'd')

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
include(${CMAKE_MODULE_PATH}/Findlibusb.cmake)
find_package(libusb REQUIRED)

file(GLOB CANDLE_SOURCE "src/*cpp")
if(CANDLE_BUILD_STATIC)
    set(CANDLE_BUILD_STATIC "TRUE")
    set(CANDLE_BUILD_SHARED "FALSE")
    add_library(candle STATIC ${CANDLE_SOURCE})
else()
    set(CANDLE_BUILD_STATIC "FALSE")
    set(CANDLE_BUILD_SHARED "TRUE")
    add_library(candle SHARED ${CANDLE_SOURCE})
endif()

message(STATUS "\nCANDLE_BUILD_SHARED: ${CANDLE_BUILD_SHARED}\nCANDLE_BUILD_STATIC: ${CANDLE_BUILD_STATIC}")

target_include_directories(candle PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(candle SYSTEM PRIVATE ${LIBUSB_INCLUDE_DIR})
target_link_libraries(candle pthread ${LIBUSB_LIBRARY})
set_target_properties(candle PROPERTIES PUBLIC_HEADER candle.hpp)
target_compile_options(candle PRIVATE -Wall -Wextra -Wpedantic)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(candle PRIVATE -g3)
else()
    target_compile_options(candle PRIVATE -O2)
endif()

string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" _ "${CANDLE_LIB_VERSION}")
target_compile_definitions(candle
    PRIVATE CANDLE_VMAJOR=${CMAKE_MATCH_1}
    PRIVATE CANDLE_VMINOR=${CMAKE_MATCH_2}
    PRIVATE CANDLE_VREVISION=${CMAKE_MATCH_3}
    PRIVATE CANDLE_VTAG=${VERSION_TAG}
)

if(MAKE_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

