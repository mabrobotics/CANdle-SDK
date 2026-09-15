# Linux ARM64 cross compilation toolchain ## sudo apt-get update sudo apt-get
# install gcc-aarch64-linux-gnu sudo apt install g++-aarch64-linux-gnu

message(STATUS "Using ARM64 cross compilation toolchain")

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Set cross compile target os
set(CROSSCOMPILING_TARGET_OS "LINUX")
set(TARGET_OS "LINUX")

# Set name if not defined as argument ##
if(NOT TOOLCHAIN_CROSS_TRIPLET)
  set(TOOLCHAIN_CROSS_TRIPLET "aarch64-linux-gnu"
  ) # may also be set to different value using cmake ..

  # -DTOOLCHAIN_CROSS_TRIPLET="aarch64-linux-gnu" ......
endif()

set(PKG_CONFIG_EXECUTABLE pkg-config)
set(ENV{PKG_CONFIG_DIR} "")
set(ENV{PKG_CONFIG_LIBDIR} "/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "/")

set(CMAKE_C_COMPILER "${TOOLCHAIN_CROSS_TRIPLET}-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_CROSS_TRIPLET}-g++")

set(CMAKE_SYSROOT "/")

set(CMAKE_LIBRARY_ARCHITECTURE ${TOOLCHAIN_CROSS_TRIPLET})

set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu /usr/lib/aarch64-linux-gnu)

list(APPEND CMAKE_SYSTEM_LIBRARY_PATH "/usr/lib/${TOOLCHAIN_CROSS_TRIPLET}")
list(APPEND CMAKE_SYSTEM_INCLUDE_PATH "/usr/include/${TOOLCHAIN_CROSS_TRIPLET}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

list(APPEND CMAKE_SYSTEM_PREFIX_PATH "/usr/${TOOLCHAIN_CROSS_TRIPLET}")
list(APPEND CMAKE_SYSTEM_LIBRARY_PATH "/usr/lib/${TOOLCHAIN_CROSS_TRIPLET}")
list(APPEND CMAKE_SYSTEM_INCLUDE_PATH "/usr/include/${TOOLCHAIN_CROSS_TRIPLET}")
