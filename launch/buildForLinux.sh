#!/bin/bash
BUILD_ROOT_DIR=./build
BUILD_DIRECTORY=${BUILD_ROOT_DIR}/linux/
start_dir=$(pwd)
base_dir="$(dirname "$(dirname "$(readlink -f "${BASH_SOURCE}")")")"
cd ${base_dir}

mkdir $BUILD_DIRECTORY -p
cd $BUILD_DIRECTORY
cmake ../..
make -j4
cpack -G DEB
chmod -R a+rw ${base_dir}/$BUILD_ROOT_DIR

cd ${start_dir}
