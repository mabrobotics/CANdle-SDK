FROM ubuntu:22.04

ARG USERNAME=mabBuildBot
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Create the user
RUN groupadd --gid $USER_GID $USERNAME
RUN useradd --uid $USER_UID --gid $USER_GID -m $USERNAME

ARG DEBIAN_FRONTEND=noninteractive

RUN apt update
RUN apt upgrade -y

RUN apt install -y wget
RUN apt install -y make
RUN apt install -y cmake
RUN apt install -y git
RUN apt install -y build-essential
RUN apt install -y libusb-1.0-0-dev
RUN apt install -y libglfw3-dev
RUN apt install -y libgtk-3-dev
RUN apt install -y libspdlog-dev
RUN apt install -y rpm
RUN apt install -y libssl-dev

RUN git config --global --add safe.directory '*'