cmake_minimum_required(VERSION 3.8)
project(shared_data)

add_library(shared_data INTERFACE)
target_include_directories(shared_data INTERFACE .)
