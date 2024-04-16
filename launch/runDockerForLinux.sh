#!/bin/bash

dir="$(dirname "$(dirname "$(readlink -f "${BASH_SOURCE}")")")"
docker run -u root \
    -v "$dir":"/candle-sdk" \
    candle-sdk-win:v1 \
    /bin/bash -c "cd /candle-sdk && ./launch/buildForLinux.sh"
    
