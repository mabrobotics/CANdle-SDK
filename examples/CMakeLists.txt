cmake_minimum_required(VERSION 3.8)

project(examples)

file(GLOB APP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cpp/*.cpp)

foreach(testsourcefile ${APP_SOURCES})
    get_filename_component(executable ${testsourcefile} NAME_WE)

    add_executable(${executable}
        ${testsourcefile}
    )

    target_link_libraries(${executable} PRIVATE
        pthread
        candle
        -static-libstdc++
        -static-libgcc
    )
endforeach()
add_subdirectory(unit_test)
