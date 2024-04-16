
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

