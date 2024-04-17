#!/bin/bash
if [[ "$EUID" != 0 ]]; then
    echo "This script must be run as root!"
    sudo -k
    if sudo false; then
        echo "Wrong password"
        exit 1
    fi
fi

dir="$(dirname "$(dirname "$(readlink -f "${BASH_SOURCE}")")")"
sudo cp $dir/CANdle_lib/launch/99-candle.rules /etc/udev/rules.d/
