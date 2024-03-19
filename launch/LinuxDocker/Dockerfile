FROM ubuntu:latest
CMD ["bash"]
ARG DEBIAN_FRONTEND=noninteractive
RUN /bin/sh -c "apt update"
RUN /bin/sh -c "apt upgrade -y"
RUN /bin/sh -c "apt install -y wget make cmake git build-essential libusb-1.0-0-dev libglfw3-dev libgtk-3-dev libspdlog-dev rpm"
RUN git config --global --add safe.directory '*'
RUN /bin/sh -c "apt install -y libssl-dev"