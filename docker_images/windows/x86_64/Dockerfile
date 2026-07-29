FROM mabroboticsofficial/mab_build_environment:v2
CMD ["bash"]
ARG DEBIAN_FRONTEND=noninteractive
RUN /bin/sh -c "apt update"
RUN /bin/sh -c "apt upgrade -y"
RUN /bin/sh -c "apt install -y wget make cmake git build-essential mingw-w64 libusb-1.0-0-dev nsis rpm pkg-config libgl1-mesa-dev libx11-dev mingw-w64-x86_64-glfw3 || apt install -y mingw-w64-common libglfw3-dev"
RUN git config --global --add safe.directory '*'
# RUN git clone https://github.com/openssl/openssl.git
# RUN /openssl/Configure --cross-compile-prefix=x86_64-w64-mingw32- mingw64 -static no-shared no-stdio no-sock
# RUN make -j
# RUN make install
