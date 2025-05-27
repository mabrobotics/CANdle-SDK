cmake_minimum_required(VERSION 3.5)

project(candlelib)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(PYTHON_BUILD)
  set(CANDLE_BUILD_STATIC ON)
endif()


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
  src/crc.cpp
  src/can_bootloader/can_bootloader.cpp
  src/communication_device/candle.cpp
  src/communication_device/candle_bootloader.cpp
  src/communication_interface/SPI.cpp
  src/communication_interface/USB.cpp
  src/MD/MD.cpp
  src/pds/pds.cpp
  src/pds/pds_module.cpp
  src/pds/pds_protocol.cpp
  src/pds/power_stage.cpp
  src/pds/brake_resistor.cpp
  src/pds/isolated_converter.cpp
)

if(UNIX)
  target_compile_definitions(candle PRIVATE UNIX)
endif()
target_include_directories(candle PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/MD
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pds
  ${CMAKE_CURRENT_SOURCE_DIR}/src/can_bootloader
  ${CMAKE_CURRENT_SOURCE_DIR}/src/communication_device
  ${CMAKE_CURRENT_SOURCE_DIR}/src/communication_interface
  ${CMAKE_CURRENT_SOURCE_DIR}/../commons/logger
  ${LIBUSB_INCLUDE_DIR}
  )
target_include_directories(candle SYSTEM PRIVATE ${LIBUSB_INCLUDE_DIR})
if(UNIX)
  target_link_libraries(candle shared_data logger pthread ${LIBUSB_LIBRARY})
else()
  target_link_libraries(candle shared_data logger pthread ${LIBUSB_WINDOWS_STATIC} -static)

endif()
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
target_link_libraries(pds_protocol_test PRIVATE logger shared_data candle)


add_unit_test_executable(candle_v2_test src/communication_device/candle_test.cpp)
target_sources(candle_v2_test PRIVATE src/communication_device/candle.cpp)
target_include_directories(candle_v2_test PRIVATE
  include
  src/communication_device
  src/communication_interface
)
target_link_libraries(candle_v2_test PRIVATE logger shared_data candle)

add_unit_test_executable(candle_bootloader_test src/communication_device/candle_bootloader_test.cpp)
target_sources(candle_bootloader_test PRIVATE src/communication_device/candle_bootloader_test.cpp)
target_include_directories(candle_bootloader_test PRIVATE
  include
  src/communication_device
  src/communication_interface
)
target_link_libraries(candle_bootloader_test PRIVATE logger shared_data candle)


add_unit_test_executable(usb_v2_test src/communication_interface/SPI_test.cpp)
target_sources(usb_v2_test PRIVATE src/communication_interface/USB.cpp)
target_include_directories(usb_v2_test PRIVATE
  include
  src/communication_interface/
)
target_link_libraries(usb_v2_test PRIVATE logger shared_data candle)

add_unit_test_executable(spi_v2_test src/communication_interface/SPI_test.cpp)
target_sources(spi_v2_test PRIVATE  src/communication_interface/SPI.cpp)
target_include_directories(spi_v2_test PRIVATE
  include
  src/communication_interface/
)
target_link_libraries(spi_v2_test PRIVATE logger shared_data candle)


add_unit_test_executable(md_v2_test src/MD/MD_test.cpp)
target_sources(md_v2_test PRIVATE  src/MD/MD.cpp)
target_include_directories(md_v2_test PRIVATE
  include
  src/MD/
)
target_link_libraries(md_v2_test PRIVATE logger shared_data candle)
