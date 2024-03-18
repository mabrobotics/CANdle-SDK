#!/bin/bash

rm -rf build
mkdir build
whoami
cd build
# cp /usr/local/lib64/libcrypto.dll.a /usr/local/lib64/libcrypto.a
# cp /usr/local/lib64/libssl.dll.a /usr/local/lib64/libssl.a
# ls /usr/local/lib64/
cmake -DPLATFORM="WIN" -DOPENSSL_ROOT_DIR="/usr/local/lib64/" -DOPENSSL_USE_STATIC_LIBS=TRUE ..
make -j4