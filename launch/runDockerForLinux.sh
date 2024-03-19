#!/bin/bash

docker run -u root \
    -v "$(pwd)/..":"/candle-sdk" \
    candle-sdk-linux:v1 \
    /bin/bash -c "cd /candle-sdk && ./launch/buildForLinux.sh"
    