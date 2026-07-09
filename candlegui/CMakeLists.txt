cmake_minimum_required(VERSION 3.15...3.25)
project(candlegui)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

add_subdirectory(lib)

add_executable(mab_tuner main.cpp) 

set(COMMON_INCLUDES
    ../candlelib/include
    ../commons/logger
    ../candlelib/src/objectDictionary
)

target_include_directories(mab_tuner PRIVATE ${COMMON_INCLUDES})

target_link_libraries(mab_tuner PRIVATE
        pthread
        candle
        -static-libstdc++
        -static-libgcc
        mabGUI
    )