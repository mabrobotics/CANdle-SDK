#!/bin/bash

start_dir=$(pwd)
base_dir="$(dirname "$(readlink -f "${BASH_SOURCE}")")"
cd ${base_dir}

docker run -u root \
    -v "$(pwd)/..":"/candle-sdk" \
    candle-sdk-linux:v1 \
    /bin/bash -c "cd /candle-sdk && ./launch/buildForLinux.sh"

cd ${start_dir}
    