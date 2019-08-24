#!/bin/bash

set -x

cd /sw/nic/buildroot

#remove the older kernel header and prepare new one
rm -rf output/linux-headers/
mkdir -p output/linux-headers/arch

#find the linux kernel source dir to prepare kernel source from
cd output/build/linux-`grep BR2_LINUX_KERNEL_VERSION .config | cut -d'"' -f2`

cp -a arch/arm64 /sw/nic/buildroot/output/linux-headers/arch
cp -a include /sw/nic/buildroot/output/linux-headers/
cp -a scripts /sw/nic/buildroot/output/linux-headers/
cp Module.symvers /sw/nic/buildroot/output/linux-headers/
cp Makefile /sw/nic/buildroot/output/linux-headers/

#remove the generated files from Linux tree before preparing tar ball
make mrproper && rm -f arch/arm64/boot/Images && rm -f usr/initramfs_data.cpio.gz
tar -czf /sw/nic/buildroot/output/build/platform-linux.tar.gz .
cd /sw/nic/buildroot/output/linux-headers/arch
ln -s arm64 aarch64
