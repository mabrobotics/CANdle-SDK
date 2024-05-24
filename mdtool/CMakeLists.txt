cmake_minimum_required(VERSION 3.5)

set(MDTOOL_VERSION 1.4.0)
set(MDTOOL_VERSION_TAG 'r')

project(mdtool)

add_compile_options(-Wall -Wextra -Wpedantic)

file(GLOB SRC "src/*.cpp")

# enable_testing()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CANDLE_BUILD_STATIC ON)
set(CANDLE_OMIT_EXAMPLES ON)

set(CMAKE_BUILD_TYPE Release)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} candle)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/candlelib/include ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/mINI/src)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "mdtool")

string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" _ "${MDTOOL_VERSION}")
target_compile_definitions(${PROJECT_NAME}
    PRIVATE MDTOOL_VMAJOR=${CMAKE_MATCH_1}
    PRIVATE MDTOOL_VMINOR=${CMAKE_MATCH_2}
    PRIVATE MDTOOL_VREVISION=${CMAKE_MATCH_3}
    PRIVATE MDTOOL_VTAG=${MDTOOL_VERSION_TAG}
)

if(MAKE_TESTS_MDTOOL)
    add_subdirectory(test)
endif()

set(CPACK_GENERATOR "DEB")
set(CPACK_PROJECT_NAME "mdtool")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MAB Robotics <contact@mabrobotics.pl>")
set(CPACK_PACKAGE_VENDOR "MAB Robotics")
set(CPACK_PACKAGE_DESCRIPTION "Console tool for configuring and setting up MD80 via CANdle")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${ARCH})
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${PROJECT_SOURCE_DIR}/template_package/postinst")

set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
include(CPack)

install(TARGETS ${PROJECT_NAME}
    DESTINATION /usr/local/bin)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/template_package/etc"
    DESTINATION /)
