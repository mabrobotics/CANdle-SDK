#!/bin/bash
if [[ "$EUID" != 0 ]]; then
    echo "This script must be run as root!"
    sudo -k
    if sudo false; then
        echo "Wrong password"
        exit 1
    fi
fi

start_dir=$(pwd)
base_dir="$(dirname "$(dirname "$(readlink -f "${BASH_SOURCE}")")")"
cd ${base_dir}

sudo cp ./candlelib/launch/99-candle.rules /etc/udev/rules.d/

cd ${start_dir}
