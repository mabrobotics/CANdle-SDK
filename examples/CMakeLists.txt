cmake_minimum_required(VERSION 3.8)

# Shared sources
set(COMMON_SOURCES
    ../candlelib/src/objectDictionary/OD.cpp
    ../candletool/src/edsParser.cpp
)

# Shared includes 
set(COMMON_INCLUDES
    ../candlelib/include
    ../commons/logger
    ../candlelib/src/objectDictionary
    ../candletool/include
     ../candletool/3rd_party/mINI/src
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
