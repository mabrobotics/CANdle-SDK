## Linux ARM64 cross compilation toolchain ##
# sudo apt-get update
# sudo apt-get install gcc-aarch64-linux-gnu
# sudo apt install g++-aarch64-linux-gnu

message(STATUS "Using ARM64 cross compilation toolchain")

set (CMAKE_SYSTEM_NAME Linux)
set (CMAKE_SYSTEM_PROCESSOR aarch64)

# Set cross compile target os
set (CROSSCOMPILING_TARGET_OS "LINUX")
set (TARGET_OS "LINUX")

## Set name if not defined as argument ##
if (NOT TOOLCHAIN_CROSS_TRIPLET)
	set(TOOLCHAIN_CROSS_TRIPLET "aarch64-linux-gnu") # may also be set to different value using cmake .. -DTOOLCHAIN_CROSS_TRIPLET="aarch64-linux-gnu" ......
endif ()

execute_process(COMMAND which "${TOOLCHAIN_CROSS_TRIPLET}-gcc" OUTPUT_VARIABLE CMAKE_C_COMPILER_NEWLINED)
execute_process(COMMAND which "${TOOLCHAIN_CROSS_TRIPLET}-g++" OUTPUT_VARIABLE CMAKE_CXX_COMPILER_NEWLINED)

string(STRIP "${CMAKE_C_COMPILER_NEWLINED}" CMAKE_C_COMPILER)
string(STRIP "${CMAKE_CXX_COMPILER_NEWLINED}" CMAKE_CXX_COMPILER)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
