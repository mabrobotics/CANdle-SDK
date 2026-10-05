# CPack configuration for the mdgui package.
#
# See the matching comment in candletool/cmake/CPackConfig.cmake: this gives
# mdgui its own CPack config/output (CPACK_OUTPUT_CONFIG_FILE below) and
# restricts it to CANDLEGUI_CPACK_COMPONENTS (built up in CMakeLists.txt
# alongside each install() call) so it doesn't clobber or bundle candletool's
# package. See the top-level CMakeLists.txt for the combined `package`
# target that invokes both projects' configs, and
# launch/buildForLinux.sh / launch/buildForWindows.sh for how each config is
# invoked directly via `cpack --config`.

set(CPACK_PROJECT_NAME "mdgui")
set(CPACK_PACKAGE_NAME mdgui)
set(CPACK_PACKAGE_FILE_NAME
    ${CPACK_PACKAGE_NAME}-${CANDLEGUI_VERSION_FULL}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}
)
set(CPACK_PACKAGE_VERSION_MAJOR ${CANDLEGUI_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${CANDLEGUI_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${CANDLEGUI_VERSION_PATCH})
set(CPACK_PACKAGE_VENDOR "MAB Robotics")
set(CPACK_PACKAGE_DESCRIPTION
    "GUI tool for MD configuration and tuning.")

if(WIN32)
    set(CPACK_MONOLITHIC_INSTALL ON)
else()
    set(CPACK_MONOLITHIC_INSTALL OFF)
endif()

if(WIN32)
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/MAB.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/MAB.ico")
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
            DetailPrint \\\"Driver installing...\\\"
            ExecWait '\\\"$INSTDIR\\\\bin\\\\candlesdk-win-driver.exe\\\"'
        ")
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
    set(CPACK_PACKAGE_EXECUTABLES "mdgui" "MDgui")
    set(CPACK_CREATE_DESKTOP_LINKS "mdgui")
elseif(UNIX)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MAB Robotics <contact@mabrobotics.pl>")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libusb-1.0-0, libgl1, libglfw3")
    set(CPACK_DEBIAN_PACKAGE_VERSION)

    if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
    elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armhf")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")
    endif()

    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/template_package/postinst")
endif()
# (a non-UNIX, non-WIN32 system already hit message(FATAL_ERROR) earlier in
# CMakeLists.txt, before this file is included)

set(CPACK_COMPONENTS_ALL ${CANDLEGUI_CPACK_COMPONENTS})
set(CPACK_INSTALL_CMAKE_PROJECTS)
foreach(_candlegui_component ${CANDLEGUI_CPACK_COMPONENTS})
    list(APPEND CPACK_INSTALL_CMAKE_PROJECTS
         "${CMAKE_BINARY_DIR};${CMAKE_PROJECT_NAME};${_candlegui_component};/")
endforeach()
set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/CPackConfig_mdgui.cmake")
include(CPack)
