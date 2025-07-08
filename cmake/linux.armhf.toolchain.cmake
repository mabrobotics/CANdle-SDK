## Linux ARM64 cross compilation toolchain ##
# sudo apt update
# sudo apt install gcc-arm-linux-gnueabihf
# sudo apt install g++-arm-linux-gnueabihf

message(STATUS "Using ARMHF cross compilation toolchain")

set (CMAKE_SYSTEM_NAME Linux)
set (CMAKE_SYSTEM_PROCESSOR armhf)

# Set cross compile target os
set (CROSSCOMPILING_TARGET_OS "LINUX")
set (TARGET_OS "LINUX")

## Set name if not defined as argument ##
if (NOT TOOLCHAIN_CROSS_TRIPLET)
	set(TOOLCHAIN_CROSS_TRIPLET "arm-linux-gnueabihf") # may also be set to different value using cmake .. -DTOOLCHAIN_CROSS_TRIPLET="aarch64-linux-gnu" ......
endif ()

execute_process(COMMAND which "${TOOLCHAIN_CROSS_TRIPLET}-gcc" OUTPUT_VARIABLE CMAKE_C_COMPILER_NEWLINED)
execute_process(COMMAND which "${TOOLCHAIN_CROSS_TRIPLET}-g++" OUTPUT_VARIABLE CMAKE_CXX_COMPILER_NEWLINED)

string(STRIP "${CMAKE_C_COMPILER_NEWLINED}" CMAKE_C_COMPILER)
string(STRIP "${CMAKE_CXX_COMPILER_NEWLINED}" CMAKE_CXX_COMPILER)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
