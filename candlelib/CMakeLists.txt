cmake_minimum_required(VERSION 3.5)

project(candlelib)

include(GNUInstallDirs)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CANDLESDK_BUILD_PYTHON)
    set(CANDLE_BUILD_STATIC ON)
endif()

find_package(Threads REQUIRED)

if(UNIX)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
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

if(UNIX)
    set(UNIX_ONLY_SOURCES src/communication_interface/SPI.cpp)
endif()

target_sources(
  candle
  PRIVATE src/crc.cpp
          src/can_bootloader/can_bootloader.cpp
          src/communication_device/candle.cpp
          src/communication_device/candle_frame_adapter.cpp
          src/communication_device/candle_bootloader.cpp
          ${UNIX_ONLY_SOURCES}
          src/communication_interface/USB.cpp
          src/logger/logger.cpp
          src/MD/MD.cpp
          src/MD/MDCO.cpp
          src/pds/pds.cpp
          src/pds/pds_module.cpp
          src/pds/pds_protocol.cpp
          src/pds/power_stage.cpp
          src/pds/brake_resistor.cpp
          src/pds/isolated_converter.cpp
          src/objectDictionary/edsParser.cpp
          src/objectDictionary/edsEntry.cpp)

if(UNIX)
    target_compile_definitions(candle PRIVATE UNIX)
endif()
target_include_directories(
  candle
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
if(UNIX)
    target_link_libraries(candle PUBLIC Threads::Threads libusb::libusb PRIVATE $<BUILD_INTERFACE:mINI>)
else()
    target_link_libraries(candle PUBLIC Threads::Threads ${LIBUSB_WINDOWS_STATIC} -static PRIVATE $<BUILD_INTERFACE:mINI>)
endif()
if(NOT SUPPRESS_WARN)
    target_compile_options(candle PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-missing-field-initializers)
else()
    target_compile_options(candle PRIVATE -w)
endif()
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

# Install headers
install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export targets for downstream usage
install(
  TARGETS candle
  EXPORT candleTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(
  EXPORT candleTargets
  FILE candleTargets.cmake
  NAMESPACE candle::
  DESTINATION ${CMAKE_INSTALL_DATADIR}/candle/cmake)

# Create and install package config files for downstream usage with find_package
include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/candleConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/candleConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/candle/cmake)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/candleConfigVersion.cmake
  VERSION ${CANDLESDK_VERSION}
  COMPATIBILITY SameMajorVersion)
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/candleConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/candleConfigVersion.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/Findlibusb.cmake
  DESTINATION ${CMAKE_INSTALL_DATADIR}/candle/cmake)

if(MAKE_TESTS)
    enable_testing()

    add_unit_test_executable(pds_protocol_test test/pds/pds_protocol_test.cpp)
    target_link_libraries(pds_protocol_test PRIVATE candle)

    add_unit_test_executable(candle_v2_test test/communication_device/candle_test.cpp)
    target_link_libraries(candle_v2_test PRIVATE candle)

    add_unit_test_executable(candle_bootloader_test
      test/communication_device/candle_bootloader_test.cpp)
    target_link_libraries(candle_bootloader_test PRIVATE candle)

    add_unit_test_executable(usb_v2_test test/communication_interface/USB_test.cpp)
    target_link_libraries(usb_v2_test PRIVATE candle)

    if(UNIX)
        add_unit_test_executable(spi_v2_test
          test/communication_interface/SPI_test.cpp)
        target_link_libraries(spi_v2_test PRIVATE candle)
    endif()

    add_unit_test_executable(md_v2_test test/MD/MD_test.cpp)
    target_link_libraries(md_v2_test PRIVATE candle)

    add_unit_test_executable(candle_frame_adapter_test
      test/communication_device/candle_frame_adapter_test.cpp)
    target_link_libraries(candle_frame_adapter_test PRIVATE candle)

    add_unit_test_executable(eds_entry_test test/objectDictionary/edsEntry_test.cpp)
    target_link_libraries(eds_entry_test PRIVATE candle)

    add_unit_test_executable(candle_frame_dto_test test/types/candle_frame_dto_test.cpp)
    target_link_libraries(candle_frame_dto_test PRIVATE candle)

    add_unit_test_executable(logger_test test/logger/logger_test.cpp)
    target_link_libraries(logger_test PRIVATE candle)

endif()
