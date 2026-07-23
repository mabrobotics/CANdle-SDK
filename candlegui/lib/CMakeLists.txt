cmake_minimum_required(VERSION 3.15...3.25)

add_library(mabGUI STATIC
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_impl_glfw.cpp
    imgui_impl_opengl3.cpp
    implot.cpp
    implot_demo.cpp
    implot_items.cpp
)

target_include_directories(mabGUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(mabGUI PRIVATE -Wno-error=deprecated-enum-enum-conversion)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW3 REQUIRED glfw3)

target_link_directories(mabGUI PRIVATE ${GLFW3_LIBRARY_DIRS})
target_link_libraries(mabGUI PRIVATE ${GLFW3_LIBRARIES} OpenGL::GL)