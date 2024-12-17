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

(See: [Installing docker on Linux(ubuntu)](https://docs.docker.com/engine/install/ubuntu/) )

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
./launch/runDockerForWindows.sh
```

### VS Code in devcontainer

1. Install "ms-vscode-remote.remote-containers" extension (from workspace recommendations)
2. Run "Reopen in container" Command in VS Code ( Press **F1** and type "**Dev Containers: reopen in container**")

Devcontainers extension will Build a docker image using [**Dockerfile**](launch/LinuxDocker/Dockerfile) for Linux, create a container and install VSCode server in the container alongside with necessary extensions.
See [**.devcontainer.json**](.devcontainer/devcontainer.json) for more details

### Cross-compile for Windows
```
sudo apt install  g++-mingw-w64-x86-64-posix
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