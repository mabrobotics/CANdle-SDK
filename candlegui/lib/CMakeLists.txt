cmake_minimum_required(VERSION 3.15...3.25)

find_package(OpenGL REQUIRED)

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

add_library(mabGUI STATIC
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_impl_glfw.cpp
    imgui_impl_opengl3.cpp
    implot.cpp
    implot_items.cpp
)

target_include_directories(mabGUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(TARGET glfw3)
    set(GLFW_TARGET glfw3)
elseif(TARGET glfw)
    set(GLFW_TARGET glfw)
else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLFW3 QUIET glfw3)
        if(GLFW3_FOUND)
            target_include_directories(mabGUI PRIVATE ${GLFW3_INCLUDE_DIRS})
            target_link_directories(mabGUI PRIVATE ${GLFW3_LIBRARY_DIRS})
            set(GLFW_TARGET ${GLFW3_LIBRARIES})
        endif()
    endif()
endif()

target_link_libraries(mabGUI PUBLIC ${GLFW_TARGET} OpenGL::GL)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mabGUI PRIVATE -Wno-error=deprecated-enum-enum-conversion)
endif()