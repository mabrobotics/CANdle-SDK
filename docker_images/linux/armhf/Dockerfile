FROM mabroboticsofficial/mab_build_environment:v3

RUN sudo dpkg --add-architecture armhf

RUN sudo sed -i -e 's/^deb http/deb [arch=amd64] http/g' /etc/apt/sources.list && \
    sudo sed -i -e 's/^deb-src http/deb-src [arch=amd64] http/g' /etc/apt/sources.list

RUN echo "deb [arch=armhf] http://ports.ubuntu.com/ubuntu-ports/ jammy main multiverse universe restricted" | sudo tee -a /etc/apt/sources.list && \
    echo "deb [arch=armhf] http://ports.ubuntu.com/ubuntu-ports/ jammy-security main multiverse universe restricted" | sudo tee -a /etc/apt/sources.list && \
    echo "deb [arch=armhf] http://ports.ubuntu.com/ubuntu-ports/ jammy-updates main multiverse universe restricted" | sudo tee -a /etc/apt/sources.list

RUN sudo apt-get update -y

RUN sudo apt-get install -y \
    gcc-arm-linux-gnueabihf \
    binutils-arm-linux-gnueabihf \
    g++-arm-linux-gnueabihf \
    libusb-1.0-0-dev:armhf \
    libgl1-mesa-dev:armhf \
    libx11-dev:armhf \
    libglfw3-dev:armhf \
    pkg-config