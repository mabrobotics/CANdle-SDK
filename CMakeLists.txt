cmake_minimum_required(VERSION 3.15...3.25)

include(git_func.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED PLATFORM)
  set(PLATFORM "native")
  message("Compiling for native system")
elseif("${PLATFORM}" STREQUAL "WIN")
  message("Cross - compiling for Windows")
  set(CMAKE_CXX_COMPILER /usr/bin/x86_64-w64-mingw32-g++-posix) #TODO: Convert this into toolchain file
  set(CMAKE_C_COMPILER /usr/bin/x86_64-w64-mingw32-gcc-posix)
  set(CMAKE_SYSTEM_NAME Windows)
endif()

set(CANDLESDK_VERSION_1 1)
set(CANDLESDK_VERSION_2 0)
set(CANDLESDK_VERSION_3 0)
set(CANDLESDK_VERSION ${CANDLESDK_VERSION_1}.${CANDLESDK_VERSION_2}.${CANDLESDK_VERSION_3})
get_tag(CANDLESDK_VERSION_TAG)

project(candlesdk VERSION ${CANDLESDK_VERSION_1}.${CANDLESDK_VERSION_2}.${CANDLESDK_VERSION_3})

message("CandleSDK version: ${CANDLESDK_VERSION} ${CANDLESDK_VERSION_TAG}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
  -Wfatal-errors
  -Wall
  -Wextra
  -Wpedantic
  -Wno-unused-parameter
  -Wno-missing-field-initializers)

if(NOT DEFINED PLATFORM)
add_compile_options(
  -Werror)
endif()

set(mINI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/mINI/inc/)

set(CANDLE_BUILD_STATIC TRUE)

macro(add_unit_test_executable test_name)
    message("Adding unit test executable ${test_name}")
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name} PRIVATE Unit_test)
    set_target_properties(${test_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/ut_runners/
        OUTPUT_NAME test_${test_name}
    )

endmacro(add_unit_test_executable)

add_subdirectory(commons)
add_subdirectory(candlelib)
if(PYTHON_BUILD)
    add_subdirectory(pycandle)
else()
    add_subdirectory(candletool)
    add_subdirectory(examples)
endif()
