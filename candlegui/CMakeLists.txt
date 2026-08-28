cmake_minimum_required(VERSION 3.15...3.25)

project(candlegui)

if(WIN32)
    enable_language(RC)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

set(CANDLEGUI_VERSION_MAJOR ${CANDLESDK_VERSION_1})
set(CANDLEGUI_VERSION_MINOR ${CANDLESDK_VERSION_2})
set(CANDLEGUI_VERSION_PATCH ${CANDLESDK_VERSION_3})
# CANDLESDK_VERSION_TAG carries literal quotes (e.g. 'x') so it also works as
# a C char-literal when used as a compile definition elsewhere; strip them
# here since CANDLEGUI_VERSION_TAG is only used as a plain string.
string(REPLACE "'" "" CANDLEGUI_VERSION_TAG "${CANDLESDK_VERSION_TAG}")

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
    logo.rc
   )

add_executable(mdgui ${CANDLEGUI_SOURCES}) 

if(TARGET glfw3)
    set(GLFW_TARGET glfw3)
elseif(TARGET glfw)
    set(GLFW_TARGET glfw)
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

set(CANDLEGUI_CPACK_COMPONENTS candlegui)

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
elseif(UNIX)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/template_package/etc/udev/rules.d/99-mdgui.rules DESTINATION "/${CMAKE_INSTALL_SYSCONFDIR}/udev/rules.d/" COMPONENT rulesgui)
    list(APPEND CANDLEGUI_CPACK_COMPONENTS rulesgui)

    target_compile_definitions(mdgui PRIVATE DEFAULT_custom_CONFIG_DIR="/${CMAKE_INSTALL_SYSCONFDIR}/custom")
    add_custom_target(mdgui_install_rules COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_BINARY_DIR}" --component rulesgui)
else()
    message(FATAL_ERROR "System not UNIX nor Windows, cannot compile!")
endif()

if(MAKE_TESTS)
    enable_testing()
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPackConfig.cmake)

