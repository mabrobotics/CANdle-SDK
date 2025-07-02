#!/bin/bash
BUILD_ROOT_DIR=./build
BUILD_DIRECTORY=${BUILD_ROOT_DIR}/linux/
start_dir=$(pwd)
base_dir="$(dirname "$(dirname "$(readlink -f "${BASH_SOURCE}")")")" # CANdle SDK directory
cd ${base_dir}

mkdir $BUILD_DIRECTORY -p
cd $BUILD_DIRECTORY
cmake ../../ -DPLATFORM="WIN"
make -j
cpack -G NSIS
chmod -R a+rw ${base_dir}/$BUILD_ROOT_DIR

cd ${start_dir}
