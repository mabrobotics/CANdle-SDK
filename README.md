# CANdle-SDK

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

To use CANdle on Windows, one must manually change USB driver for CANdle, to WinUSB:

1. Download and run [Zadig 2.8](https://github.com/pbatard/libwdi/releases/download/v1.5.0/zadig-2.8.exe)
2. Options -> List all devices
3. Select `MD USB-TO-CAN` from drop down menu
4. Changed driver type to `libusb-win32`
5. Click `Replace Driver`
6. Wait for installation to finish.

## Build

### Linux based OS

Building for Linux system:

```
./launch/buildForLinux.sh
```

#### Using Docker (only on x86_64 architecture)

(See: [Installing docker on Linux(ubuntu)](https://docs.docker.com/engine/install/ubuntu/) )

Run:

```
./launch/runDockerForLinux86-64.sh
```

### Cross-compile for Windows

#### Using Docker

```
./launch/runDockerForWindows.sh
```

### Natively build on Windows

Using powershell run

```
./launch/buildForWindows.bat
```

### Compiling Python module

Dependencies are listed inside pyproject.toml

To compile for the current system run build command inside repository using your preferred python binary, for eg. `python -m build`.

To compile against multiple versions of libc and python use:

```
./launch/pythonBuildWheel.sh
```

To install use pip install on the desired wheel, for eg. `python -m pip install ./dist/pycandlemab-1.7.0-cp310-cp310-linux_x86_64.whl` for CPython 3.10, glibc and x86-64 arch.

## Including CANdle-SDK in your projects

Best way to include CANdle-SDK in your code is to include it as a git submodule and include it in you CMakeLists.txt like this:

```
git submodule add git@github.com:mabrobotics/CANdle-SDK.git
git submodule update --init --recursive
```

You can than use candlelib as a library to link against your executables.

Below examplary CMakeLists.txt:

```
cmake_minimum_required(VERSION 3.15)

project(myCandleProject)

add_subdirectory(CANdle-SDK) # added as a submodule
add_executable(myCandleProject main.cpp)
target_link_libraries(myCandleProject candle)
```

## Documentation

For more information check out our documentation page [here](https://mabrobotics.github.io/MD80-x-CANdle-Documentation/software_package/Software.html).
