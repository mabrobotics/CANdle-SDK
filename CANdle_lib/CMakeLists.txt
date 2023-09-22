cmake_minimum_required(VERSION 3.8)

if(MAKE_TESTS)
    add_subdirectory(test)
    return()
endif()

if(NOT DEFINED PLATFORM)
    set(PLATFORM "native")
    message("Compiling for native system")
elseif("${PLATFORM}" STREQUAL "WIN")
    message("Cross - compiling for Windows")
    set(CMAKE_CXX_COMPILER /usr/bin/x86_64-w64-mingw32-g++-posix)
    set(CMAKE_C_COMPILER /usr/bin/x86_64-w64-mingw32-gcc-posix)
    set(CMAKE_RC_COMPILER /usr/bin/x86_64-w64-mingw32-windres)
    set(CMAKE_SYSTEM_NAME Windows)
endif()

project(CANdle_lib)

set(CANDLE_LIB 0.0.1)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall
    -Wextra
	-Wpedantic
    -Wno-unused-parameter
    -Wno-missing-field-initializers)

if(UNIX)
    set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
    include(${CMAKE_MODULE_PATH}/Findlibusb.cmake)
    find_package(libusb REQUIRED)
endif()

if(WIN32)
    set(LIBUSB_WINDOWS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libusb/lib/windows/libusb-1.0.dll)
    set(LIBUSB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libusb/inc/libusb-1.0)
endif()

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/BusHandler/UsbHandler.cpp)

set(EXECUTABLE ${CMAKE_PROJECT_NAME})

add_executable(${EXECUTABLE} ${PROJECT_SOURCES})

target_include_directories(${EXECUTABLE} SYSTEM PRIVATE
    ${LIBUSB_INCLUDE_DIR})

target_include_directories(${EXECUTABLE} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/BusHandler/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CircularBuffer/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Commons/)

if(UNIX)
    target_link_libraries(${EXECUTABLE} ${LIBUSB_LIBRARY} ${SPDLOG_LINUX} pthread)
elseif(WIN32)
    target_link_libraries(${EXECUTABLE} ${LIBUSB_WINDOWS} ${SPDLOG_WINDOWS} -static ssp -static-libstdc++ -static-libgcc)
endif()

if(WIN32)
    add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${LIBUSB_WINDOWS}
        ${CMAKE_BINARY_DIR}/libusb-1.0.dll)
endif()
