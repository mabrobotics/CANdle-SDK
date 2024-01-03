cmake_minimum_required(VERSION 3.8)

if(MAKE_TESTS)
    add_subdirectory(test)
    return()
endif()

project(MDtool)
set(EXECUTABLE MDtool)
set(MDtool_VERSION 0.0.1)

find_package(OpenSSL REQUIRED)
set(mINI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/mINI/inc/)
set(CLI11_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/CLI11/inc/)
set(SPDLOG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mdtool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MD80_Flasher/Downloader/Downloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MD80_Flasher/Checksum/Checksum.cpp)

add_executable(${EXECUTABLE} ${PROJECT_SOURCES})

target_include_directories(${EXECUTABLE} SYSTEM PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/
${CMAKE_CURRENT_SOURCE_DIR}/src/MD80_Flasher/
${CMAKE_CURRENT_SOURCE_DIR}/src/MD80_Flasher/Downloader/
${CMAKE_CURRENT_SOURCE_DIR}/src/MD80_Flasher/BinaryParser/)

target_include_directories(${EXECUTABLE} SYSTEM PRIVATE
    ${CLI11_INCLUDE_DIR}
    ${mINI_INCLUDE_DIR}
    ${SPDLOG_INCLUDE_DIR})

target_link_libraries(${EXECUTABLE} pthread CANdle_lib -static-libstdc++ -static-libgcc OpenSSL::SSL OpenSSL::Crypto)