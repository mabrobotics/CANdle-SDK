cmake_minimum_required(VERSION 3.15...3.25)

include(git_func.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CANDLESDK_VERSION_1 1)
set(CANDLESDK_VERSION_2 3)
set(CANDLESDK_VERSION_3 0)
set(CANDLESDK_VERSION
    ${CANDLESDK_VERSION_1}.${CANDLESDK_VERSION_2}.${CANDLESDK_VERSION_3})
add_compile_definitions(CANDLESDK_VERSION="${CANDLESDK_VERSION}")
get_tag(CANDLESDK_VERSION_TAG)

project(
  candlesdk
  VERSION ${CANDLESDK_VERSION_1}.${CANDLESDK_VERSION_2}.${CANDLESDK_VERSION_3})

message("====================")
message("CandleSDK version: ${CANDLESDK_VERSION} ${CANDLESDK_VERSION_TAG}")
message("Available flags:")
message(
  "CANDLESDK_BUILD_PYTHON (default OFF) - set buildsystem to build python library shared object"
)
message(
  "CANDLE_BUILD_STATIC (default ON) -  set buildsystem to link against dependencies statically and build CANdle as .a"
  "(has no effect on windows library and python builds)")
message("MAKE_TESTS (default ON) - enables/disables some of the unit tests")
message("====================")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wfatal-errors -Wall -Wextra -Wpedantic
                    -Wno-unused-parameter -Wno-missing-field-initializers)

if(NOT WIN32)
  add_compile_options(-Werror)
  add_compile_definitions(WIN32_MEAN_AND_LEAN) # This define is needed for
  # windows.h to exclude some less used APIs
endif()

set(mINI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/mINI/inc/)
if(NOT DEFINED CANDLE_BUILD_STATIC)
  set(CANDLE_BUILD_STATIC TRUE)
endif()

macro(add_unit_test_executable test_name)
  message("Adding unit test executable ${test_name}")
  add_executable(${test_name} ${ARGN})
  target_link_libraries(${test_name} PRIVATE Unit_test)
  set_target_properties(
    ${test_name}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/ut_runners/
               OUTPUT_NAME test_${test_name})

endmacro(add_unit_test_executable)

add_subdirectory(commons)
add_subdirectory(candlelib)
if(CANDLESDK_BUILD_PYTHON)
  add_subdirectory(pycandle)
else()
  add_subdirectory(candletool)
  add_subdirectory(examples)
endif()
