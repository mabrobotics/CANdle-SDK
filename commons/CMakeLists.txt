cmake_minimum_required(VERSION 3.8)
project(commons)

add_library(logger STATIC logger/logger.cpp)
target_include_directories(logger PRIVATE logger)
target_compile_options(logger PRIVATE -fPIC)
