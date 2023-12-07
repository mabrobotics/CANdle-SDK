cmake_minimum_required(VERSION 3.8)

set(EXECUTABLE MDtool)
set(MDtool 0.0.1)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

add_executable(${EXECUTABLE} ${PROJECT_SOURCES} )

target_link_libraries(${EXECUTABLE} pthread CANdle_lib -static-libstdc++ -static-libgcc)