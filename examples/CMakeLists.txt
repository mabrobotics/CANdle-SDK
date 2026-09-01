cmake_minimum_required(VERSION 3.8...3.25)
project(examples)

file(GLOB APP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cpp/*.cpp)

foreach(testsourcefile ${APP_SOURCES})
    get_filename_component(executable ${testsourcefile} NAME_WE)

    add_executable(${executable}
        ${testsourcefile}
        ${COMMON_SOURCES}
    )

    target_include_directories(${executable} PRIVATE ../commons/logger)

    target_link_libraries(${executable} PRIVATE
        candle
        -static-libstdc++
        -static-libgcc
    )
endforeach()

add_subdirectory(unit_test)
