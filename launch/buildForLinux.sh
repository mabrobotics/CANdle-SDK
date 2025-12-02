#!/bin/bash
set -e


SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIRECTORY="$PROJECT_ROOT/build"

cd "$PROJECT_ROOT"

mkdir -p "$BUILD_DIRECTORY"
cd "$BUILD_DIRECTORY"

chmod -R a+rw "$BUILD_DIRECTORY"

cmake ..
chmod -R a+rw "$BUILD_DIRECTORY"

make -j4
chmod -R a+rw "$BUILD_DIRECTORY"

cpack -G DEB
chmod -R a+rw "$BUILD_DIRECTORY"
