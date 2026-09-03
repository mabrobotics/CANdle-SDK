cmake_minimum_required(VERSION 3.5...3.25)
project(candlelib)

include(GNUInstallDirs)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CANDLELIB_VERSION_MAJOR ${CANDLESDK_VERSION_1})
set(CANDLELIB_VERSION_MINOR ${CANDLESDK_VERSION_2})
set(CANDLELIB_VERSION_PATCH ${CANDLESDK_VERSION_3})
set(CANDLELIB_VERSION_TAG ${CANDLESDK_VERSION_TAG})

# CANDLELIB_VERSION_TAG carries literal quotes (e.g. 'x') so it also works as
# a C char-literal when substituted into the CANDLELIBL_VTAG compile
# definition below. Strip them for uses as a plain string, such as the
# package version string / CPack file name.
string(REPLACE "'" "" CANDLELIB_VERSION_TAG_STR "${CANDLELIB_VERSION_TAG}")
set(CANDLELIB_VERSION_FULL
    "${CANDLELIB_VERSION_MAJOR}.${CANDLELIB_VERSION_MINOR}.${CANDLELIB_VERSION_PATCH}-${CANDLELIB_VERSION_TAG_STR}"
)

# GoogleTest
set(INSTALL_GTEST OFF)
add_subdirectory(3rd_party/googletest)

# UNIT TEST
add_library(Unit_test INTERFACE)
target_sources(Unit_test INTERFACE test/unit_test_runner.cpp)
target_link_libraries(Unit_test INTERFACE gmock)

find_package(Threads REQUIRED)

if(CANDLESDK_BUILD_PYTHON)
    set(CANDLE_BUILD_STATIC ON)
endif()

if(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0)
endif()

if(WIN32)
    set(LIBUSB_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/libusb/lib/windows/libusb-1.0.a)
    set(LIBUSB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/libusb/inc/libusb-1.0)
    set(LIBUSB_WINDOWS_DYNAMIC ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/libusb/lib/windows/libusb-1.0.dll)
endif()

add_library(candle)
add_library(candle::candle ALIAS candle)

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

target_compile_features(candle PUBLIC cxx_std_20)

if(UNIX)
    target_compile_definitions(candle PRIVATE UNIX)
endif()

target_include_directories(candle PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if(UNIX)
    target_link_libraries(candle PUBLIC Threads::Threads PkgConfig::LIBUSB PRIVATE mINI)
else()
    target_link_libraries(candle PUBLIC Threads::Threads ${LIBUSB_LIBRARIES} PRIVATE mINI -static)
    target_include_directories(candle PUBLIC $<BUILD_INTERFACE:${LIBUSB_INCLUDE_DIR}>)
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

string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" _ ${CANDLESDK_VERSION})

target_compile_definitions(
    candle
    PRIVATE CANDLE_VMAJOR=${CANDLELIB_VERSION_MAJOR}
    PRIVATE CANDLE_VMINOR=${CANDLELIB_VERSION_MINOR}
    PRIVATE CANDLE_VREVISION=${CANDLELIB_VERSION_PATCH}
    PRIVATE CANDLE_VTAG=${CANDLELIB_VERSION_TAG})

set(CANDLELIB_CPACK_COMPONENTS candlelib)

if(UNIX)
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/candle/modules
        COMPONENT candlelib)
endif()

install(
    TARGETS candle mINI
    EXPORT candleTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT candlelib)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT candlelib)

install(
    EXPORT candleTargets
    FILE candleTargets.cmake
    NAMESPACE candle::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/candle
    COMPONENT candlelib)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/candleConfigVersion.cmake
    VERSION ${CANDLESDK_VERSION}
    COMPATIBILITY SameMajorVersion)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/candleConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/candleConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/candle)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/candleConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/candleConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/candle
    COMPONENT candlelib)

if(MAKE_TESTS)
    enable_testing()

    add_unit_test_executable(logger_test test/logger/logger_test.cpp)
    target_include_directories(logger_test PRIVATE include src/logger)
    target_link_libraries(logger_test PRIVATE candle)

    add_unit_test_executable(candle_frame_dto_test
        test/candle_frame_dto_test.cpp)
    target_link_libraries(candle_frame_dto_test PRIVATE candle)

    add_unit_test_executable(pds_protocol_test test/pds/pds_protocol_test.cpp)
    target_include_directories(pds_protocol_test PRIVATE include src/pds)
    target_link_libraries(pds_protocol_test PRIVATE candle)

    add_unit_test_executable(candle_v2_test
        test/communication_device/candle_test.cpp)
    target_include_directories(
        candle_v2_test
        PRIVATE
        include
        src/communication_device
        src/communication_interface)
    target_link_libraries(candle_v2_test PRIVATE candle)

    add_unit_test_executable(candle_bootloader_test
        test/communication_device/candle_bootloader_test.cpp)
    target_include_directories(
        candle_bootloader_test PRIVATE include src/communication_device
        src/communication_interface)
    target_link_libraries(candle_bootloader_test PRIVATE candle)

    add_unit_test_executable(usb_v2_test test/communication_interface/USB_test.cpp)
    target_include_directories(usb_v2_test PRIVATE include
        src/communication_interface/)
    target_link_libraries(usb_v2_test PRIVATE candle)

    if(UNIX)
        add_unit_test_executable(spi_v2_test
            test/communication_interface/SPI_test.cpp)
        target_include_directories(spi_v2_test PRIVATE include
            src/communication_interface/)
        target_link_libraries(spi_v2_test PRIVATE candle)
    endif()

    add_unit_test_executable(md_v2_test test/MD/MD_test.cpp)
    target_include_directories(md_v2_test PRIVATE include src/MD/)
    target_link_libraries(md_v2_test PRIVATE candle)

    add_unit_test_executable(
        candle_frame_adapter_test
        test/communication_device/candle_frame_adapter_test.cpp)
    target_include_directories(candle_frame_adapter_test
        PRIVATE include src src/communication_device)
    target_link_libraries(candle_frame_adapter_test PRIVATE candle)

    add_unit_test_executable(eds_entry_test test/objectDictionary/edsEntry_test.cpp)
    target_include_directories(eds_entry_test PRIVATE src/objectDictionary)
    target_link_libraries(eds_entry_test PRIVATE candle)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPackConfig.cmake)
