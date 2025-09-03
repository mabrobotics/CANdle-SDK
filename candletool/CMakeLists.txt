cmake_minimum_required(VERSION 3.5)

include(GNUInstallDirs)

message(CMAKE_INSTALL_SYSCONFDIR: ${CMAKE_INSTALL_SYSCONFDIR})

set(CANDLETOOL_VERSION_MAJOR ${CANDLESDK_VERSION_1})
set(CANDLETOOL_VERSION_MINOR ${CANDLESDK_VERSION_2})
set(CANDLETOOL_VERSION_PATCH ${CANDLESDK_VERSION_3})
set(CANDLETOOL_VERSION_TAG "r")

set(CANDLETOOL_VERSION_FULL
    "${CANDLETOOL_VERSION_MAJOR}.${CANDLETOOL_VERSION_MINOR}.${CANDLETOOL_VERSION_PATCH}-${CANDLETOOL_VERSION_TAG}"
)

add_subdirectory(3rd_party/CLI11)

set(CANDLETOOL_SOURCES
  objectDictionary/OD.cpp
  src/candle_cli.cpp
  src/canLoader.cpp
  src/web_file_module/curl_handler.cpp
  src/web_file_module/flasher.cpp
  src/candletoolCO.cpp
  src/edsParser.cpp
  src/md_cli.cpp
  src/mdco_cli.cpp
  src/mab_crc.cpp
  src/mabFileParser.cpp
  src/main.cpp
  src/pds_cli.cpp
  src/utilities.cpp)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_executable(candletool ${CANDLETOOL_SOURCES})

target_link_libraries(candletool candle logger)

target_include_directories(
  candletool
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
         ${CMAKE_CURRENT_SOURCE_DIR}/objectDictionary 
         ${CMAKE_CURRENT_SOURCE_DIR}/src/web_file_module
         ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/mINI/src)


target_compile_options(candletool PRIVATE -g)

set_target_properties(candletool PROPERTIES OUTPUT_NAME "candletool")

target_compile_definitions(
  candletool
  PRIVATE CANDLETOOL_VMAJOR=${CANDLETOOL_VERSION_MAJOR}
          CANDLETOOL_VMINOR=${CANDLETOOL_VERSION_MINOR}
          CANDLETOOL_VREVISION=${CANDLETOOL_VERSION_PATCH}
          CANDLETOOL_VTAG=${CANDLETOOL_VERSION_TAG})

target_link_libraries(candletool CLI11)

# add_unit_test_executable(pds_cli_test tests/pds_cli_test.cpp)
# target_include_directories(pds_cli_test PRIVATE include)
# target_link_libraries(pds_cli_test PRIVATE CLI11 shared_data) target_

add_custom_command(
  TARGET candletool
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/candletool/
    ${CMAKE_CURRENT_BINARY_DIR}/
  COMMENT "Copying configs...")

# package / installer
set(CPACK_PROJECT_NAME "candletool")
set(CPACK_PACKAGE_NAME candletool)
set(CPACK_PACKAGE_FILE_NAME
    ${CPACK_PACKAGE_NAME}-${CANDLETOOL_VERSION_FULL}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}
)
set(CPACK_PACKAGE_VERSION_MAJOR ${CANDLETOOL_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${CANDLETOOL_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${CANDLETOOL_VERSION_PATCH})
set(CPACK_PACKAGE_VENDOR "MAB Robotics")
set(CPACK_PACKAGE_DESCRIPTION
    "Console tool for MAB devices ecosystem configuration")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
set(CPACK_MONOLITHIC_INSTALL ON)

if(WIN32)
  add_executable(candle-win-driver IMPORTED
                 ${CMAKE_CURRENT_SOURCE_DIR}/tools/candlesdk-win-driver.exe)

  install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/tools/candlesdk-win-driver.exe
          TYPE BIN)
  install(TARGETS candletool RUNTIME COMPONENT candletool)

  install(
    DIRECTORY
      ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/candletool/config/motors
    DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/candletool/config"
    COMPONENT _configs)

  install(
    FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/candletool/config/candletool.ini
      ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/candletool/config/web_files_metadata.ini
    DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/candletool/config"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
                WORLD_WRITE
    COMPONENT _configs)
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_MODIFY_PATH ON)
  set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/MAB.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/MAB.ico")
  target_link_libraries(
    candletool
    -static
    candle
    -static-libstdc++
    -static-libgcc
    -static
    logger)

  add_compile_definitions(
    DEFAULT_CANDLETOOL_CONFIG_DIR="${CMAKE_INSTALL_SYSCONFDIR}/candletool/config"
  )
elseif(UNIX)

  install(TARGETS candletool RUNTIME COMPONENT candletool)

  install(
    DIRECTORY
      ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/candletool/config/motors
    DESTINATION "/${CMAKE_INSTALL_SYSCONFDIR}/candletool/config"
    COMPONENT _configs)

  install(
    FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/candletool/config/candletool.ini
      ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/candletool/config/web_files_metadata.ini
    DESTINATION "/${CMAKE_INSTALL_SYSCONFDIR}/candletool/config"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
                WORLD_WRITE
    COMPONENT _configs)

  set(CPACK_GENERATOR "DEB")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MAB Robotics <contact@mabrobotics.pl>")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS libusb-1.0-0)
  set(CPACK_DEBIAN_PACKAGE_VERSION)

  if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")

    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")

  elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armhf")

    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")

  endif()
  message(CPACK_DEBIAN_PACKAGE_ARCHITECTURE =
          ${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})

  set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
      "${CMAKE_CURRENT_SOURCE_DIR}/template_package/postinst")

  install(
    FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/udev/rules.d/99-candletool.rules
    DESTINATION "/${CMAKE_INSTALL_SYSCONFDIR}/udev/rules.d/")

  install(
    FILES
      "${PROJECT_SOURCE_DIR}/template_package/etc/candletool/config/candletool.ini"
      "${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/candletool/config/web_files_metadata.ini"
    DESTINATION "/etc/candletool/"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
                WORLD_WRITE
    EXCLUDE_FROM_ALL)
  add_compile_definitions(DEFAULT_CANDLETOOL_CONFIG_DIR="/etc/candletool/")
else()
  message(FATAL_ERROR "System not unix nor windows, can not compile!")
  message()

endif()

if(MAKE_TESTS)
  enable_testing()
endif()

include(CPack)
