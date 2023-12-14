include(FetchContent)

project(MDTOOL_test)

set(EXECUTABLE ${CMAKE_PROJECT_NAME})

enable_testing()

find_package(OpenSSL REQUIRED)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)

add_library(GTest::GTest INTERFACE IMPORTED)

include_directories(${EXECUTABLE}
    ${CMAKE_SOURCE_DIR}/MDtool/src/
    ${CMAKE_SOURCE_DIR}/MDtool/src/MD80_Flasher/BinaryParser/)

include_directories(${EXECUTABLE} SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/MDtool/third_party/mINI/inc/)

# set(SOURCES   XXX)

target_link_libraries(GTest::GTest INTERFACE gtest_main gmock gmock_main)

add_executable(${EXECUTABLE} main.cpp
    BinaryParserTest.cpp
    ${SOURCES})

add_compile_options(-Wall -Wextra -Wpedantic)

target_link_libraries(${EXECUTABLE}
    PRIVATE
    GTest::GTest
    OpenSSL::SSL OpenSSL::Crypto)

    add_custom_command(TARGET ${EXECUTABLE} PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/MDtool/test/BinaryParserFiles $<TARGET_FILE_DIR:${EXECUTABLE}>)