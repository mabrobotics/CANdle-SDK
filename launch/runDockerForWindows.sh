#!/bin/bash

start_dir=$(pwd)
base_dir="$(dirname "$(readlink -f "${BASH_SOURCE}")")"
cd ${base_dir}

image=candle-sdk-win:v1

docker image inspect ${image} &> /dev/null
inspect=$?
if [ ${inspect} != 0 ]; then
    echo "${image} not found locally."
    read -p "Build it? [y/n]" -n 1 -r
    echo    # (optional) move to a new line
    if [[ ! $REPLY =~ ^[Yy]$ ]] then
        [[ "$0" = "$BASH_SOURCE" ]] && exit 1 || return 1
    fi
    docker build ./WindowsDocker/ -t ${image}
fi

docker run -u root \
    -v "$(pwd)/..":"/candle-sdk" \
    ${image} \
    /bin/bash -c "cd /candle-sdk && ./launch/buildForWindows.sh"

cd ${start_dir}
    
