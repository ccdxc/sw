#!/bin/bash

set -x

cd /sw/nic/buildroot

OUT_DIR=$1
#remove the older kernel header and prepare new one
rm -rf ${OUT_DIR}/linux-headers/
mkdir -p ${OUT_DIR}/linux-headers/arch

#find the linux kernel source dir to prepare kernel source from
cd ${OUT_DIR}/build/linux-`grep BR2_LINUX_KERNEL_VERSION .config | cut -d'"' -f2`

cp -a arch/arm64 /sw/nic/buildroot/${OUT_DIR}/linux-headers/arch
cp -a include /sw/nic/buildroot/${OUT_DIR}/linux-headers/
cp -a scripts /sw/nic/buildroot/${OUT_DIR}/linux-headers/
cp Module.symvers /sw/nic/buildroot/${OUT_DIR}/linux-headers/
cp Makefile /sw/nic/buildroot/${OUT_DIR}/linux-headers/

#remove the generated files from Linux tree before preparing tar ball
make mrproper && rm -f arch/arm64/boot/Images && rm -f usr/initramfs_data.cpio.gz
tar -czf /sw/nic/buildroot/${OUT_DIR}/build/platform-linux.tar.gz .
cd /sw/nic/buildroot/${OUT_DIR}/linux-headers/arch
ln -s arm64 aarch64
