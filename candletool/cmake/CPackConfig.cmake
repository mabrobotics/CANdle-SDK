# CPack configuration for the candletool package.
#
# CANdle-SDK bundles multiple installable executables (candletool, mdgui).
# Each gets its own CPack config/output (CPACK_OUTPUT_CONFIG_FILE below) so
# that including this doesn't clobber the one generated for candlegui (both
# would otherwise write to the same default
# ${CMAKE_BINARY_DIR}/CPackConfig.cmake). CPACK_INSTALL_CMAKE_PROJECTS is
# restricted to CANDLETOOL_CPACK_COMPONENTS (built up in CMakeLists.txt
# alongside each install() call) so this package doesn't bundle mdgui's
# install rules too. See the top-level CMakeLists.txt for the combined
# `package` target that invokes both projects' configs, and
# launch/buildForLinux.sh / launch/buildForWindows.sh for how each config is
# invoked directly via `cpack --config`.

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
set(CPACK_MONOLITHIC_INSTALL OFF)

if(WIN32)
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_MODIFY_PATH ON)
  set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/MAB.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/MAB.ico")
elseif(UNIX)
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
endif()

set(CPACK_COMPONENTS_ALL ${CANDLETOOL_CPACK_COMPONENTS})
set(CPACK_INSTALL_CMAKE_PROJECTS)
foreach(_candletool_component ${CANDLETOOL_CPACK_COMPONENTS})
  list(APPEND CPACK_INSTALL_CMAKE_PROJECTS
       "${CMAKE_BINARY_DIR};${CMAKE_PROJECT_NAME};${_candletool_component};/")
endforeach()
set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/CPackConfig_candletool.cmake")
include(CPack)
