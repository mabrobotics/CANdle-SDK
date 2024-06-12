cmake_minimum_required(VERSION 3.5)

project(mdtool)

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
target_compile_options(mdtool PRIVATE -g)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "mdtool")

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE MDTOOL_VMAJOR=${CANDLESDK_VERSION_1}
  PRIVATE MDTOOL_VMINOR=${CANDLESDK_VERSION_2}
  PRIVATE MDTOOL_VREVISION=${CANDLESDK_VERSION_3}
  PRIVATE MDTOOL_VTAG=${CANDLESDK_VERSION_TAG})

if(MAKE_TESTS_MDTOOL)
  add_subdirectory(test)
endif()

# package / installer
set(CPACK_PROJECT_NAME "mdtool")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "MAB Robotics")
set(CPACK_PACKAGE_DESCRIPTION
    "Console tool for configuring and setting up MAB MD motor controllers via CANdle."
)
if(WIN32)
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
  target_link_libraries(
    ${PROJECT_NAME}
    -static
    candle
    -static-libstdc++
    -static-libgcc
    -static
    logger)
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${INSTALL_PATH}
                                          COMPONENT mdtool)
  install(
    DIRECTORY "./template_package/etc/mdtool/"
    DESTINATION "bin/config/"
    COMPONENT Configuration)
  install(
    FILES "${PROJECT_SOURCE_DIR}/template_package/mdtool.ini"
    DESTINATION "bin/config/"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
                WORLD_WRITE)
elseif(UNIX)
  set(CPACK_GENERATOR "DEB")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MAB Robotics <contact@mabrobotics.pl>")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS libusb-1.0-0-dev)
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${ARCH})
  set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
      "${PROJECT_SOURCE_DIR}/template_package/postinst")
  install(TARGETS ${PROJECT_NAME} DESTINATION /usr/local/bin)
  install(DIRECTORY "${PROJECT_SOURCE_DIR}/template_package/etc" DESTINATION /)
  install(
    FILES "${PROJECT_SOURCE_DIR}/template_package/mdtool.ini"
    DESTINATION "/etc/mdtool/"
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
                WORLD_WRITE)
endif()

include(CPack)
