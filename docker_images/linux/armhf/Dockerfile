FROM mabroboticsofficial/mab_build_environment:v2

RUN echo "deb [arch=armhf] http://ports.ubuntu.com/ noble main multiverse universe" | sudo tee -a /etc/apt/sources.list
RUN echo "deb [arch=armhf] http://ports.ubuntu.com/ noble-security main multiverse universe" | sudo tee -a /etc/apt/sources.list
RUN echo "deb [arch=armhf] http://ports.ubuntu.com/ noble-backports main multiverse universe" | sudo tee -a /etc/apt/sources.list
RUN echo "deb [arch=armhf] http://ports.ubuntu.com/ noble-updates main multiverse universe" | sudo tee -a /etc/apt/sources.list

# exit 0 because update will fail due to incomplete backport repository
RUN sudo apt update ; exit 0

RUN sudo apt install -y gcc-arm-linux-gnueabihf
RUN sudo apt install -y binutils-arm-linux-gnueabihf
RUN sudo apt install -y g++-arm-linux-gnueabihf

RUN sudo dpkg --add-architecture armhf

RUN sudo apt install -y libusb-1.0-0-dev:armhf
