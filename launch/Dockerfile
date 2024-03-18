FROM ubuntu:latest
CMD ["bash"]
ARG DEBIAN_FRONTEND=noninteractive
RUN /bin/sh -c "apt update"
RUN /bin/sh -c "apt upgrade -y"
RUN /bin/sh -c "apt install -y wget make cmake git build-essential mingw-w64 libusb-1.0-0-dev libglfw3-dev libgtk-3-dev libspdlog-dev nsis rpm"
RUN git config --global --add safe.directory '*'
RUN git clone https://github.com/openssl/openssl.git
# RUN bash -c "apt-get update && apt-get install -y libssl-dev"
RUN /openssl/Configure --cross-compile-prefix=x86_64-w64-mingw32- mingw64 -static no-shared no-stdio no-sock
RUN make -j4
RUN make install