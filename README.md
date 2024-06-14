# CANdle-SDK
*WARNING: This software is still in Beta phase. Performance may vary.*

A toolkit for developing applications using MAB Robotics MD motor controllers and actuators.

## Dependencies & Requirements
### Linux
```
sudo apt install build-essential git cmake libusb-1.0-0-dev
```

### Windows
Package requires w64devkit to build. It can be automatically downloaded and configured using:
```
launch/buildForWindows.bat
```
To use CANdle on Windows, one must manually change USB driver for CANdle, to libUSB:
1. Download and run [Zadig 2.8](https://github.com/pbatard/libwdi/releases/download/v1.5.0/zadig-2.8.exe)
2. Options -> List all devices
3. Select `MD USB-TO-CAN` from drop down menu
4. Changed driver type to `libusb-win32`
5. Click `Replace Driver`
6. Wait for installation to finish.


## Build
Building for Linux system:
```
./launch/buildForLinux.sh
```
### Using Docker
First build docker container:
```
docker build launch/dockerForLinux/ -t candle-sdk-linux:v1

OR (for Windows cross compilation)
docker build launch/dockerForWindows/ -t candle-sdk-windows:v1
```
and run 
```
./launch/runDockerForLinux.sh
OR
./lauch/runDockerForWindows.sh
```
### Cross-compile for Windows
```
sudo apt install  g++-mingw-w64-x86-64-posix
```

### Natively build on Windows
Using powershell run
```
./launch/buildForWindows.bat
```