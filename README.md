
# Building

## Requirements
- docker (optional - for building)

## Linux (native)
```
./launch/buildForLinux.sh
```

## Linux (docker)
```
sudo docker build ./launch/LinuxDocker/
sudo docker build ./launch/runDockerForLinux.sh
```

## Windows (cross-compile via docker)
```
sudo docker build ./launch/WindowsDocker/
./launch/runDockerForWindows.sh
```


# Running

## Windows
### Requirements
- Driver replacement via Zadig

#### USB driver replacement
    Download Zadig 2.8 or later [ADD LINK HERE]
    Options -> List all devices
    Select `MD USB-TO-CAN` from drop down menu
    Changed driver type to `libusb-win32`
    Click `Replace Driver`
    Wait for installation to finish

