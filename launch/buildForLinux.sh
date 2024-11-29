#!/bin/bash

BUILD_DIRECTORY=build/linux/
start_dir=$(pwd)
base_dir="$(dirname "$(dirname "$(readlink -f "${BASH_SOURCE}")")")"
cd ${base_dir}

mkdir $BUILD_DIRECTORY -p
cd $BUILD_DIRECTORY
pwd
cmake ../..
make -j4
cpack -G DEB -DCPACK_INSTALL_CMAKE_PROJECTS=ALL

cd ${start_dir}
