cmake_minimum_required(VERSION 3.8)

if(NOT DEFINED PLATFORM)
    set(PLATFORM "native")
    message("Compiling for native system")
elseif("${PLATFORM}" STREQUAL "WIN")
    message("Cross - compiling for Windows")
    set(CMAKE_CXX_COMPILER /usr/bin/x86_64-w64-mingw32-g++-posix)
    set(CMAKE_C_COMPILER /usr/bin/x86_64-w64-mingw32-gcc-posix)
    set(CMAKE_SYSTEM_NAME Windows)
endif()

project(CANdle_SDK)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)

if(WIN32 AND "${PLATFORM}" STREQUAL "native")
    set(LLVM_DIR "C:/msys64/mingw64/bin")
    set(CMAKE_PREFIX_PATH ${LLVM_DIR} ${CMAKE_PREFIX_PATH})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
    set(CMAKE_AR "C:/msys64/mingw64/bin/llvm-ar.exe")
endif()

add_compile_options(-Wall
    -municode
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
    -Wno-missing-field-initializers)

add_subdirectory(CANdle_lib)
add_subdirectory(MDtool)
add_subdirectory(Examples)
