cmake_minimum_required(VERSION 3.8)

project(examples)

# if(WIN32)
#     set(SPDLOG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/)
# endif()

set(mINI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/mINI/inc/)

file(GLOB APP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cpp/*.cpp)
foreach(testsourcefile ${APP_SOURCES})
    get_filename_component(executable ${testsourcefile} NAME_WE)
    add_executable(${executable} ${testsourcefile})
    target_include_directories(${executable} PRIVATE "../candlelib/include")
    target_include_directories(${executable} PRIVATE "../commons/logger")
    target_link_libraries(${executable} pthread candle -static-libstdc++ -static-libgcc)
endforeach(testsourcefile ${APP_SOURCES})

add_subdirectory(unit_test)
