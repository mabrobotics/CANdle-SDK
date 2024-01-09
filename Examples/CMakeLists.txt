cmake_minimum_required(VERSION 3.8)

project(Examples)
set(EXECUTABLE Examples)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/high_spped_com.cpp)

if(WIN32)
    set(SPDLOG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/)
endif()

add_executable(${EXECUTABLE} ${PROJECT_SOURCES})

# target_include_directories(${EXECUTABLE} SYSTEM PRIVATE
# ${CMAKE_CURRENT_SOURCE_DIR}/
# ${CMAKE_CURRENT_SOURCE_DIR}/src/MD80_Flasher/
# ${CMAKE_CURRENT_SOURCE_DIR}/src/MD80_Flasher/Downloader/
# ${CMAKE_CURRENT_SOURCE_DIR}/src/MD80_Flasher/BinaryParser/)

target_include_directories(${EXECUTABLE} SYSTEM PRIVATE
#     ${CLI11_INCLUDE_DIR}
#     ${mINI_INCLUDE_DIR}
    ${SPDLOG_INCLUDE_DIR})

target_link_libraries(${EXECUTABLE} pthread CANdle_lib -static-libstdc++ -static-libgcc)