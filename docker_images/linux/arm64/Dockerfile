FROM mabroboticsofficial/mab_build_environment:v3

RUN sudo dpkg --add-architecture arm64

RUN sudo sed -i -e 's/^deb http/deb [arch=amd64] http/g' /etc/apt/sources.list && \
    sudo sed -i -e 's/^deb-src http/deb-src [arch=amd64] http/g' /etc/apt/sources.list

RUN echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports/ jammy main multiverse universe restricted" | sudo tee -a /etc/apt/sources.list && \
    echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports/ jammy-security main multiverse universe restricted" | sudo tee -a /etc/apt/sources.list && \
    echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports/ jammy-updates main multiverse universe restricted" | sudo tee -a /etc/apt/sources.list

RUN sudo apt-get update -y

RUN sudo apt-get install -y \
    gcc-aarch64-linux-gnu \
    binutils-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    libusb-1.0-0-dev:arm64 \
    libgl1-mesa-dev:arm64 \
    libx11-dev:arm64 \
    libglfw3-dev:arm64 \
    pkg-config