cmake_minimum_required(VERSION 3.8)

project(Examples)

if(WIN32)
    set(SPDLOG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/)
endif()

file(GLOB APP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
foreach(testsourcefile ${APP_SOURCES})
    get_filename_component(executable ${testsourcefile} NAME_WE)
    add_executable(${executable} ${testsourcefile})
    target_include_directories(${executable} SYSTEM PRIVATE ${SPDLOG_INCLUDE_DIR})
    target_link_libraries(${executable} pthread CANdle_lib -static-libstdc++ -static-libgcc)
endforeach(testsourcefile ${APP_SOURCES})


