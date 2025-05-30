cmake_minimum_required(VERSION 3.5)

include(GNUInstallDirs)

message(CMAKE_INSTALL_SYSCONFDIR: ${CMAKE_INSTALL_SYSCONFDIR})

set(CANDLETOOL_VERSION_MAJOR 0)
set(CANDLETOOL_VERSION_MINOR 9)
set(CANDLETOOL_VERSION_PATCH 4)
set(CANDLETOOL_VERSION_TAG "beta")

set(CANDLETOOL_VERSION_FULL "${CANDLETOOL_VERSION_MAJOR}.${CANDLETOOL_VERSION_MINOR}.${CANDLETOOL_VERSION_PATCH}-${CANDLETOOL_VERSION_TAG}")

add_subdirectory(3rd_party/CLI11)

set(CANDLETOOL_SOURCES

  src/candletool.cpp
  src/canLoader.cpp
  src/mab_crc.cpp
  src/mabFileParser.cpp
  src/main.cpp
  src/pds_cli.cpp
  src/utilities.cpp
  src/ui.cpp

)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


add_executable(candletool ${CANDLETOOL_SOURCES})

target_link_libraries(candletool candle logger)

target_include_directories(candletool PUBLIC

    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/mINI/src

)

target_compile_options(candletool PRIVATE -g)

set_target_properties(candletool PROPERTIES OUTPUT_NAME "candletool")

target_compile_definitions(candletool PRIVATE
    CANDLETOOL_VMAJOR=${CANDLETOOL_VERSION_MAJOR}
    CANDLETOOL_VMINOR=${CANDLETOOL_VERSION_MINOR}
    CANDLETOOL_VREVISION=${CANDLETOOL_VERSION_PATCH}
    CANDLETOOL_VTAG=${CANDLETOOL_VERSION_TAG}
)

target_link_libraries(candletool CLI11)

# add_unit_test_executable(pds_cli_test tests/pds_cli_test.cpp)
# target_include_directories(pds_cli_test PRIVATE include)
# target_link_libraries(pds_cli_test PRIVATE CLI11 shared_data)
# target_

add_custom_command(TARGET candletool
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/candletool/ ${CMAKE_CURRENT_BINARY_DIR}/
  COMMENT "Copying configs..."
  )

# package / installer
set(CPACK_PROJECT_NAME "candletool")
set(CPACK_PACKAGE_NAME candletool)
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CANDLETOOL_VERSION_FULL}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
set(CPACK_PACKAGE_VERSION_MAJOR ${CANDLETOOL_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${CANDLETOOL_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${CANDLETOOL_VERSION_PATCH})
set(CPACK_PACKAGE_VENDOR "MAB Robotics")
set(CPACK_PACKAGE_DESCRIPTION "Console tool for MAB devices ecosystem configuration")

if(WIN32)

  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
  set(CPACK_NSIS_MODIFY_PATH ON)
  target_link_libraries(
    candletool
    -static
    candle
    -static-libstdc++
    -static-libgcc
    -static
    logger)

elseif(UNIX)

  install(TARGETS candletool
          RUNTIME
          COMPONENT candletool)

  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/candletool/config/motors
          DESTINATION "/${CMAKE_INSTALL_SYSCONFDIR}/candletool/config"
          COMPONENT _configs)

  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/candletool/config/candletool.ini
          DESTINATION "/${CMAKE_INSTALL_SYSCONFDIR}/candletool/config"
          PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_WRITE
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
  message(CPACK_DEBIAN_PACKAGE_ARCHITECTURE = ${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})

  set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/template_package/postinst")


  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/udev/rules.d/99-candletool.rules
          DESTINATION "/${CMAKE_INSTALL_SYSCONFDIR}/udev/rules.d/")

  install(FILES "${PROJECT_SOURCE_DIR}/template_package/etc/candletool/config/candletool.ini"
          DESTINATION "/etc/candletool/"
          PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
          WORLD_WRITE EXCLUDE_FROM_ALL)

endif()

include(CPack)
