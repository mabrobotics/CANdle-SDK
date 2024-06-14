#!/bin/bash

BUILD_DIRECTORY="build/windows"

start_dir=$(pwd)
base_dir="$(dirname "$(dirname "$(readlink -f "${BASH_SOURCE}")")")"
cd ${base_dir}

mkdir $BUILD_DIRECTORY -p
cd $BUILD_DIRECTORY
cmake ../../ -DPLATFORM="WIN"
make -j4
cpack -G NSIS -DCPACK_INSTALL_CMAKE_PROJECTS=ALL

cd ${start_dir}
