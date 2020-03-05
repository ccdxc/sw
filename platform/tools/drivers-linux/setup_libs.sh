#!/bin/bash

exists()
{
  command -v "$1" >/dev/null 2>&1
}

# Ugh. libnl3-devel is required for rdma-core, but Oracle
# has moved it to the 'optional' repo.
yum_repo()
{
    cat <<-EOF > /etc/yum.repos.d/oraclelinux-optional-ol7.repo
	[ol7_optional]
	name=Oracle Linux \$releasever Optional Packages (\$basearch)
	baseurl=https://yum\$ociregion.oracle.com/repo/OracleLinux/OL7/optional/latest/\$basearch/
	gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-oracle
	gpgcheck=1
	enabled=1
	EOF
}

if exists yum; then
  if grep -q oracle /etc/os-release ; then
    yum_repo
  fi
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
    ninja-build \
    pandoc \
    pkgconfig \
    python-docutils \
    valgrind-devel \
    iperf3 \
    vim \
    sshpass \
    sysfsutils \
    net-tools \
    tcpdump \
    psmisc \
    hping3 \
    nmap
  if [[ ! -e /usr/bin/ninja ]]; then
      ln -s /usr/bin/ninja-build /usr/bin/ninja
  fi

else
  apt-get update
  apt-get install -y \
    build-essential \
    libtool \
    automake \
    autoconf \
    cmake \
    gcc \
    libelf-dev \
    libudev-dev \
    libnl-3-dev \
    libnl-route-3-dev \
    ninja-build \
    pandoc \
    pkg-config \
    python-docutils \
    valgrind \
    psmisc \
    hping3 \
    nmap
fi
