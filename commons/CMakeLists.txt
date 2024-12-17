cmake_minimum_required(VERSION 3.8)
project(commons)

add_subdirectory(shared_data)

add_library(logger STATIC logger/logger.cpp)
target_include_directories(logger PUBLIC logger)
target_compile_options(logger PRIVATE -fPIC)

set(INSTALL_GTEST OFF)
add_subdirectory(googletest)
add_subdirectory(unit_test_runner)

add_unit_test_executable(logger_test logger/logger_test.cpp)
target_link_libraries(logger_test PUBLIC logger)