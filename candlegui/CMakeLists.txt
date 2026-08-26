cmake_minimum_required(VERSION 3.15...3.25)

project(candlegui)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

set(CANDLEGUI_VERSION_MAJOR ${CANDLESDK_VERSION_1})
set(CANDLEGUI_VERSION_MINOR ${CANDLESDK_VERSION_2})
set(CANDLEGUI_VERSION_PATCH ${CANDLESDK_VERSION_3})
set(CANDLEGUI_VERSION_TAG "r")

set(CANDLEGUI_VERSION_FULL
    "${CANDLEGUI_VERSION_MAJOR}.${CANDLEGUI_VERSION_MINOR}.${CANDLEGUI_VERSION_PATCH}-${CANDLEGUI_VERSION_TAG}"
)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_package(glfw3 QUIET)

if(NOT TARGET glfw AND NOT TARGET glfw3)
    include(FetchContent)
    FetchContent_Declare(
        glfw
        URL https://github.com/glfw/glfw/archive/refs/tags/3.4.zip
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
endif()

add_library(imgui STATIC
    lib/imgui.cpp
    lib/imgui_draw.cpp
    lib/imgui_tables.cpp
    lib/imgui_widgets.cpp
    lib/imgui_impl_glfw.cpp
    lib/imgui_impl_opengl3.cpp
    lib/implot.cpp
    lib/implot_items.cpp
)

set(CANDLEGUI_SOURCES
    src/main.cpp 
    src/GUI.cpp 
    src/HardwareCandle.cpp
   )

add_executable(mdgui ${CANDLEGUI_SOURCES}) 

if(TARGET glfw3)
    set(GLFW_TARGET glfw3)
elseif(TARGET glfw)
    set(GLFW_TARGET glfw)
    target_compile_options(glfw PRIVATE -Wno-error)
    get_target_property(is_imported glfw IMPORTED)
    if(NOT is_imported)
        target_compile_options(glfw PRIVATE -Wno-error)
    endif()
else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLFW3 QUIET glfw3)
        if(GLFW3_FOUND)
            target_include_directories(imgui PRIVATE ${GLFW3_INCLUDE_DIRS})
            target_link_directories(imgui PRIVATE ${GLFW3_LIBRARY_DIRS})
            set(GLFW_TARGET ${GLFW3_LIBRARIES})
        endif()
    endif()
endif()

target_link_libraries(mdgui
        Threads::Threads
        candle
        imgui
        ${GLFW_TARGET}
        OpenGL::GL
        ${CMAKE_DL_LIBS}
        )
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_link_libraries(imgui PUBLIC ${GLFW_TARGET} OpenGL::GL)

target_include_directories(mdgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)

target_compile_options(mdgui PRIVATE -g -Wno-unused-parameter -Wno-missing-field-initializers)
if(WIN32)
  target_compile_options(mdgui PRIVATE -Wa,-mbig-obj)
endif()
if(SUPPRESS_WARN)
  target_compile_options(mdgui PRIVATE -w)
endif()

target_compile_definitions(
  mdgui
  PRIVATE CANDLEGUI_VMAJOR=${CANDLEGUI_VERSION_MAJOR}
          CANDLEGUI_VMINOR=${CANDLEGUI_VERSION_MINOR}
          CANDLEGUI_VREVISION=${CANDLEGUI_VERSION_PATCH}
          CANDLEGUI_VTAG=${CANDLEGUI_VERSION_TAG})

# package / installer
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
set(CPACK_MONOLITHIC_INSTALL OFF)

if(WIN32)
    install(TARGETS mdgui RUNTIME DESTINATION . COMPONENT candlegui)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/fonts" DESTINATION . COMPONENT candlegui)
else()
    install(TARGETS mdgui RUNTIME DESTINATION bin COMPONENT candlegui)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/fonts" DESTINATION share/mdgui COMPONENT candlegui)
    
endif()

if(WIN32)
    add_executable(candlesdk-win-driver IMPORTED)
    set_property(TARGET candlesdk-win-driver PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/tools/candlesdk-win-driver.exe")
   
    install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/tools/candlesdk-win-driver.exe DESTINATION bin COMPONENT candlegui)
    
    target_compile_definitions(mdgui PRIVATE DEFAULT_custom_CONFIG_DIR="${CMAKE_INSTALL_SYSCONFDIR}/custom/config")

  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_MODIFY_PATH ON)
  set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/MAB.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/MAB.ico")
  set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        DetailPrint \\\"Driver installing...\\\"
        ExecWait '\\\"$INSTDIR\\\\bin\\\\candlesdk-win-driver.exe\\\"'
    ")
elseif(UNIX)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/udev/rules.d/99-mdgui.rules DESTINATION "/${CMAKE_INSTALL_SYSCONFDIR}/udev/rules.d/" COMPONENT rulesgui)

    target_compile_definitions(mdgui PRIVATE DEFAULT_custom_CONFIG_DIR="/${CMAKE_INSTALL_SYSCONFDIR}/custom")
    add_custom_target(mdgui_install_rules COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_BINARY_DIR}" --component rulesgui)

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

else()
    message(FATAL_ERROR "System not UNIX nor Windows, cannot compile!")
endif()

if(MAKE_TESTS)
    enable_testing()
endif()

include(CPack)




