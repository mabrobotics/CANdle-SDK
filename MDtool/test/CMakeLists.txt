include(FetchContent)

project(MDTOOL_test)

set(EXECUTABLE run_tests)

enable_testing()

set(CMAKE_BUILD_TYPE Debug)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)

add_library(GTest::GTest INTERFACE IMPORTED)

set(mINI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/mINI/inc/)
set(SPDLOG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/)

include_directories(${EXECUTABLE}
    ${CMAKE_SOURCE_DIR}/MDtool/src/
    ${CMAKE_SOURCE_DIR}/MDtool/src/Flashers/BinaryParser/
    ${CMAKE_SOURCE_DIR}/MDtool/src/ConfigParser/
    ${CMAKE_SOURCE_DIR}/CANdle_lib/src/CanopenStack/ObjectDictionary/
    ${CMAKE_SOURCE_DIR}/CANdle_lib/src/MD80)


include_directories(${EXECUTABLE} SYSTEM PRIVATE
    ${mINI_INCLUDE_DIR}
    ${SPDLOG_INCLUDE_DIR})

target_link_libraries(GTest::GTest INTERFACE gtest_main gmock gmock_main)

add_executable(${EXECUTABLE} main.cpp
    BinaryParserTest.cpp
    ConfigParserTest.cpp)

add_compile_options(-Wall -Wextra -Wpedantic)

target_link_libraries(${EXECUTABLE}
    PRIVATE
    GTest::GTest)

add_custom_command(TARGET ${EXECUTABLE} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/MDtool/test/TestFiles $<TARGET_FILE_DIR:${EXECUTABLE}>)
