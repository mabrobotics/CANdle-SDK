# CANdle-SDK

CANdle-SDK is a multi-platform software development kit (SDK) for creating MD driver control scripts. 

## Building
### Requirements
- docker (optional - for building)

### Linux (native)
```
./launch/buildForLinux.sh
```

### Linux (docker)
```
sudo docker build ./launch/LinuxDocker/
./launch/runDockerForLinux.sh
```

### Windows (cross-compile via docker)
```
sudo docker build ./launch/WindowsDocker/
./launch/runDockerForWindows.sh
```


## Running

### Windows
#### Requirements
- Driver replacement via Zadig

##### USB driver replacement
1. Download [Zadig 2.8 or later](https://github.com/pbatard/libwdi/releases/download/v1.5.0/zadig-2.8.exe)
2. Options -> List all devices
3. Select `MD USB-TO-CAN` from drop down menu
4. Changed driver type to `libusb-win32`
5. Click `Replace Driver`
6. Wait for installation to finish

