cmake_minimum_required(VERSION 3.5)

project(candlelib)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(UNIX)
  set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
  include(${CMAKE_MODULE_PATH}/Findlibusb.cmake)
  find_package(libusb REQUIRED)
endif()
if(WIN32)
  set(LIBUSB_WINDOWS_DYNAMIC
      ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libusb/lib/windows/libusb-1.0.dll)
  set(LIBUSB_WINDOWS_STATIC
      ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libusb/lib/windows/libusb-1.0.a)
  set(LIBUSB_INCLUDE_DIR
      ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libusb/inc/libusb-1.0)
endif()

if(CANDLE_BUILD_STATIC)
  set(CANDLE_BUILD_STATIC "TRUE")
  set(CANDLE_BUILD_SHARED "FALSE")
  add_library(candle STATIC ${CANDLE_SOURCE})
else()
  set(CANDLE_BUILD_STATIC "FALSE")
  set(CANDLE_BUILD_SHARED "TRUE")
  add_library(candle SHARED ${CANDLE_SOURCE})
endif()

message(
  STATUS
    "\nCANDLE_BUILD_SHARED: ${CANDLE_BUILD_SHARED}\nCANDLE_BUILD_STATIC: ${CANDLE_BUILD_STATIC}"
)

target_sources(candle PRIVATE
  src/candle.cpp
  src/bus.cpp
  src/crc.cpp
  src/md80.cpp
  src/pds/pds.cpp
  src/pds/pds_module.cpp
  src/pds/pds_protocol.cpp
  src/pds/power_stage.cpp
  src/pds/brake_resistor.cpp
  src/pds/isolated_converter.cpp
  src/register.cpp
  src/usbDevice.cpp
)

if(UNIX)
  target_sources(candle PRIVATE src/uartDevice.cpp src/spiDevice.cpp)
endif()
target_include_directories(candle PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pds
  ${CMAKE_CURRENT_SOURCE_DIR}/../commons/logger
  )
target_include_directories(candle SYSTEM PRIVATE ${LIBUSB_INCLUDE_DIR})
if(UNIX)
  target_link_libraries(candle logger pthread ${LIBUSB_LIBRARY})
else()
  target_link_libraries(candle logger pthread ${LIBUSB_WINDOWS_STATIC} -static)

endif()
set_target_properties(candle PROPERTIES PUBLIC_HEADER candle.hpp)
target_compile_options(candle PRIVATE -Wall -Wextra -Wpedantic)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(candle PRIVATE -g3)
else()
  target_compile_options(candle PRIVATE -O2)
endif()

string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" _ "${CANDLESDK_VERSION}")
target_compile_definitions(
  candle
  PRIVATE CANDLE_VMAJOR=${CMAKE_MATCH_1}
  PRIVATE CANDLE_VMINOR=${CMAKE_MATCH_2}
  PRIVATE CANDLE_VREVISION=${CMAKE_MATCH_3}
  PRIVATE CANDLE_VTAG=${CANDLESDK_VERSION_TAG})

if(MAKE_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

add_unit_test_executable(pds_protocol_test test/pds_protocol_test.cpp)
target_sources(pds_protocol_test PRIVATE src/pds/pds_protocol.cpp)
target_include_directories(pds_protocol_test PRIVATE
  include
  src/pds
)
target_link_libraries(pds_protocol_test PRIVATE logger)