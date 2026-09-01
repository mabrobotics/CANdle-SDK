cmake_minimum_required(VERSION 3.5)
project(mINI CXX)
set(CMAKE_CXX_STANDARD 17)

# Check GCC version and add -lstdc++fs if necessary
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)

    if(GCC_VERSION VERSION_LESS 9.1)
        link_libraries("-lstdc++fs")
    endif()
endif()
