cmake_minimum_required(VERSION 3.5)

project(candletool VERSION ${CANDLESDK_VERSION_1}.${CANDLESDK_VERSION_2}.${CANDLESDK_VERSION_3})

file(GLOB SRC "src/*.cpp")
# enable_testing()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} candle logger)
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
         ${CMAKE_CURRENT_SOURCE_DIR}/../candlelib/include
         ${CMAKE_CURRENT_SOURCE_DIR}/../commons/logger
         ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/mINI/src
         ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/CLI11/inc)
target_compile_options(candletool PRIVATE -g)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "candletool")

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE CANDLETOOL_VMAJOR=${CANDLESDK_VERSION_1}
  PRIVATE CANDLETOOL_VMINOR=${CANDLESDK_VERSION_2}
  PRIVATE CANDLETOOL_VREVISION=${CANDLESDK_VERSION_3}
  PRIVATE CANDLETOOL_VTAG=${CANDLESDK_VERSION_TAG})

  set(DEB_GENERATOR_SCRIPT ${CMAKE_SOURCE_DIR}/create_deb_file.sh)
  set(DEB_CTRL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/deb/control)

  add_custom_command(TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/udev ${CMAKE_CURRENT_BINARY_DIR}/
  COMMAND ${DEB_GENERATOR_SCRIPT} ${PROJECT_NAME} ${DEB_CTRL_FILE} $<TARGET_FILE:${PROJECT_NAME}>
  )

if(MAKE_TESTS_CANDLETOOL)
  add_subdirectory(test)
endif()

add_custom_command(TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/candletool/ ${CMAKE_CURRENT_BINARY_DIR}/
  COMMENT "Copying configs..."
  )

# package / installer
set(CPACK_PROJECT_NAME "candletool")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
set(CPACK_PACKAGE_VENDOR "MAB Robotics")
set(CPACK_PACKAGE_DESCRIPTION
    "Console tool for configuring and setting up MAB MD motor controllers via CANdle."
)
if(WIN32)
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
  set(CPACK_NSIS_MODIFY_PATH ON)
  target_link_libraries(
    ${PROJECT_NAME}
    -static
    candle
    -static-libstdc++
    -static-libgcc
    -static
    logger)
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${INSTALL_PATH}
                                          COMPONENT candletool)
  install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/candletool/config/motors"
    DESTINATION "bin/config"
    COMPONENT _configs)
  install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/candletool/config/candletool.ini"
    DESTINATION "bin/config/"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
                WORLD_WRITE
    COMPONENT _configs)
elseif(UNIX)
  set(CPACK_GENERATOR "DEB")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MAB Robotics <contact@mabrobotics.pl>")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS libusb-1.0-0)
  set(CPACK_DEBIAN_PACKAGE_VERSION)
  if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
  endif()
  set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
      "${PROJECT_SOURCE_DIR}/template_package/postinst")
  install(TARGETS ${PROJECT_NAME} DESTINATION /usr/local/bin EXCLUDE_FROM_ALL)
  install(DIRECTORY "${PROJECT_SOURCE_DIR}/template_package/etc" DESTINATION / EXCLUDE_FROM_ALL)
  install(
    FILES "${PROJECT_SOURCE_DIR}/template_package/etc/candletool/config/candletool.ini"
    DESTINATION "/etc/candletool/"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
                WORLD_WRITE EXCLUDE_FROM_ALL)
endif()

include(CPack)
