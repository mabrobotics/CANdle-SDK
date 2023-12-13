cmake_minimum_required(VERSION 3.8)

set(EXECUTABLE MDtool)
set(MDtool 0.0.1)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MD80_Flasher/Downloader/Downloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MD80_Flasher/Checksum/Checksum.cpp)

add_executable(${EXECUTABLE} ${PROJECT_SOURCES})

target_include_directories(${EXECUTABLE} SYSTEM PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/
${CMAKE_CURRENT_SOURCE_DIR}/src/MD80_Flasher/
${CMAKE_CURRENT_SOURCE_DIR}/src/MD80_Flasher/Downloader/
${CMAKE_CURRENT_SOURCE_DIR}/third_party
${CMAKE_CURRENT_SOURCE_DIR}/third_party/CLI11/include)

target_link_libraries(${EXECUTABLE} pthread CANdle_lib -static-libstdc++ -static-libgcc)