#!/bin/bash

set -e
set -x

if [ "$PACKER_BUILDER_TYPE" != "virtualbox-iso" ]; then
  exit 0
fi

sudo yum install -y net-tools
sudo yum install -y tcpdump
sudo yum install -y wget

sudo yum group install -y "Development Tools"

# Install necessary packages for DPDK
sudo yum install -y gcc gcc-c++ CUnit-devel libaio-devel openssl-devel vim pciutils libtool curl unzip openssl numactl-devel python-paramiko
sudo yum install -y libibverbs-devel librdmacm-devel
sudo yum install -y kernel-devel glibc.i686 libgcc.i686 libstdc++.i686 glibc-devel.i686

exit 0
