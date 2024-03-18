#!/bin/bash

docker run -u root \
    -v "$(pwd)/..":"/candle-sdk" \
    candle-sdk:v1 \
    /bin/bash -c "cd /candle-sdk && ./launch/buildForWindows.sh"
    