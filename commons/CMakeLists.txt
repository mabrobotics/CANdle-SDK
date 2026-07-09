cmake_minimum_required(VERSION 3.8)
project(commons)

set(INSTALL_GTEST OFF)
add_subdirectory(googletest)
add_subdirectory(mINI)
add_subdirectory(unit_test_runner)
