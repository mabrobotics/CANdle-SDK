cmake_minimum_required(VERSION 3.15)

project(pyCandle)

set(CMAKE_BUILD_TYPE Release)

find_package(Python COMPONENTS Interpreter REQUIRED)
add_subdirectory(3rd_party/pybind11)
message(${CMAKE_SOURCE_DIR})
pybind11_add_module(pyCandle MODULE src/bindings.cpp)

target_link_libraries(pyCandle PUBLIC candle)
install(TARGETS pyCandle LIBRARY DESTINATION .)
