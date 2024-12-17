cmake_minimum_required(VERSION 3.20)


    add_library(Unit_test INTERFACE)
    target_sources(Unit_test INTERFACE unit_test_runner.cpp)
    target_link_libraries(Unit_test INTERFACE gmock)
