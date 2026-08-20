#!/bin/bash
set -e

echo "Installing dependencies..."
sudo apt-get update
sudo apt-get install -y libgl-dev libglfw3-dev cmake build-essential
sudo apt-get install -y libusb-1.0-0-dev
echo "Dependencies installed."