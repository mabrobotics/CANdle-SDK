cmake_minimum_required(VERSION 3.15...3.25)

project(ImGuiLib)

add_library(imgui STATIC
    src/imgui.cpp
    src/implot_items.cpp
    src/imgui_draw.cpp
    src/imgui_tables.cpp
    src/implot.cpp
    src/imgui_widgets.cpp
    src/imgui_impl_opengl3.cpp
    src/imgui_impl_glfw.cpp)

target_include_directories(imgui
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)