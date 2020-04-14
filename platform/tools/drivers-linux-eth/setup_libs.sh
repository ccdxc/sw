#!/bin/bash
set -x

exists()
{
  command -v "$1" >/dev/null 2>&1
}

if exists yum; then
	installer=yum
elif exists apt-get; then
	installer=apt-get
elif exists zypper; then
	installer=zypper
else
	echo "DIDNT FIND THE INSTALLER"
fi

 $installer install -y \
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

 if [ "$installer" = "apt-get" ]; then
	 $installer install -y build-essential
 elif [ "$installer" = "zypper" ]; then
	 $installer install -y kernel-devel
 fi
