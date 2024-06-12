#!/usr/bin/env bash
sudo ./launch/buildForLinux.sh
sudo apt remove mdtool* -y
sudo apt install ./build/linux/mdtool-0.1.1-Linux.deb

