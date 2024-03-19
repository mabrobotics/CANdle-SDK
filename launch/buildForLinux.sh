#!/bin/bash

BUILD_DIRECTORY=buildLinux

rm -rf $BUILD_DIRECTORY
mkdir $BUILD_DIRECTORY
cd $BUILD_DIRECTORY
cmake ..
make -j4