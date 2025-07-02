#!/bin/bash
set -e
BUILD_DIRECTORY=build
start_dir=$(pwd)
base_dir="$(dirname "$(dirname "$(readlink -f "${BASH_SOURCE}")")")"
cd ${base_dir}

mkdir $BUILD_DIRECTORY -p
cd $BUILD_DIRECTORY
chmod -R a+rw ${base_dir}/${BUILD_DIRECTORY}
cmake ..
chmod -R a+rw ${base_dir}/${BUILD_DIRECTORY}
make -j4
chmod -R a+rw ${base_dir}/${BUILD_DIRECTORY}
cpack -G DEB
chmod -R a+rw ${base_dir}/${BUILD_DIRECTORY}

cd ${start_dir}
