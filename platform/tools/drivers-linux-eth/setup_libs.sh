#!/bin/bash

 exists()
{
  command -v "$1" >/dev/null 2>&1
}

 if exists yum; then
  yum install -y \
    libtool \
    automake \
    autoconf \
    cmake \
    gcc \
    libnl-devel \
    libnl3-devel \
    libudev-devel \
    make \
    pkgconfig \
    valgrind-devel \
    iperf3 \
    vim \
    sshpass \
    sysfsutils \
    net-tools \
    tcpdump \
    psmisc \
    hping3

 else
   apt-get install -y \
    build-essential \
    libtool \
    automake \
    autoconf \
    cmake \
    gcc \
    libudev-dev \
    libnl-3-dev \
    libnl-route-3-dev \
    ninja-build \
    pkg-config \
    valgrind \
    psmisc \
    hping3
fi
