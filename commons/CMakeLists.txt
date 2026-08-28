cmake_minimum_required(VERSION 3.8...3.25)
project(commons)

add_subdirectory(shared_data)

set(INSTALL_GTEST OFF)
add_subdirectory(googletest)
add_subdirectory(unit_test_runner)

if(MAKE_TESTS)
  add_unit_test_executable(candle_frame_dto_test
    candle_frame_dto_test/candle_frame_dto_test.cpp)
  target_link_libraries(candle_frame_dto_test PUBLIC shared_data)
endif()
