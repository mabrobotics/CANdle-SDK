#!/bin/bash

start_dir=$(pwd)
base_dir="$(dirname "$(readlink -f "${BASH_SOURCE}")")"
cd ${base_dir}

docker run -u root \
    -v "$(pwd)/..":"/candle-sdk" \
    candle-sdk-win:v1 \
    /bin/bash -c "cd /candle-sdk && ./launch/buildForWindows.sh"

cd ${start_dir}
    
