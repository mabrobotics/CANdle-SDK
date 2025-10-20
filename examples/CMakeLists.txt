cmake_minimum_required(VERSION 3.8)

# Sources shared by multiple targets
set(COMMON_SOURCES
    ../candlelib/src/objectDictionary/OD.cpp
    ../candlelib/src/objectDictionary/edsParser.cpp
)

# Include directories shared by multiple targets
set(COMMON_INCLUDES
    ../candlelib/include
    ../commons/logger
    ../candlelib/src/objectDictionary
)


project(examples)

file(GLOB APP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cpp/*.cpp)

foreach(testsourcefile ${APP_SOURCES})
    get_filename_component(executable ${testsourcefile} NAME_WE)

    add_executable(${executable}
        ${testsourcefile}
        ${COMMON_SOURCES}
    )

    target_include_directories(${executable} PRIVATE ${COMMON_INCLUDES})

    target_link_libraries(${executable} PRIVATE
        pthread
        candle
        -static-libstdc++
        -static-libgcc
    )
endforeach()
add_subdirectory(unit_test)
