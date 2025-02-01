cmake_minimum_required(VERSION 3.8)

project(candlegui)

find_package(glfw3 REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(IMGUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_opengl3.cpp)
set(IMPLOT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/implot/implot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/implot/implot_items.cpp
    )

file(GLOB srcs src/*.cpp)
add_executable(${PROJECT_NAME}
    main.cpp
    ${srcs}
    ${IMGUI_SOURCES}
    ${IMPLOT_SOURCES}
    )
target_include_directories(${PROJECT_NAME} PRIVATE SYSTEM
    src 
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/backends/
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/implot/
    ${CMAKE_CURRENT_SOURCE_DIR}/../candlelib
    )
    # ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/GLFW/inc/
    # ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/GLFW/inc/GLFW/)
target_link_libraries(${PROJECT_NAME} PRIVATE
    dl GL glfw candle
)

target_compile_options(candlegui PRIVATE
    -Wall
    -Wfatal-errors
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
    -Wno-missing-field-initializers)

