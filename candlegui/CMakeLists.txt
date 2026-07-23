cmake_minimum_required(VERSION 3.15...3.25)
project(candlegui)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

add_subdirectory(lib)

add_executable(mdgui main.cpp) 

set(COMMON_INCLUDES
    ../candlelib/include
    ../commons/logger
    ../candlelib/src/objectDictionary
)

target_include_directories(mdgui PRIVATE ${COMMON_INCLUDES})

target_link_libraries(mdgui PRIVATE
        pthread
        candle
        -static-libstdc++
        -static-libgcc
        mabGUI
    )
