#!/bin/bash

start_dir=$(pwd)
base_dir="$(dirname "$(readlink -f "${BASH_SOURCE}")")"
cd ${base_dir}

image=mabroboticsofficial/mab_build_environment_windows_cross-compile:v2

docker image inspect ${image} &> /dev/null
inspect=$?
if [ ${inspect} != 0 ]; then
    echo "${image} not found locally."
    docker pull ${image}
fi

if docker run \
    -u root \
    -v "$(pwd)/..":"/candle-sdk" \
    ${image} \
    /bin/bash -c "cd /candle-sdk && ./launch/buildForWindows.sh" ; then
    echo "Build successful."
else
    echo "Build failed."
    exit 1
fi

cd ${start_dir}
