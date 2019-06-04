#!/bin/sh
sudo dmesg -c > /dev/null
sudo kldunload pencake
sudo kldunload sonic
cd /tmp/git/pensando/sw/storage/offload && OS_DIR=/usr/src/freebsd ./freebsd_build.sh && sudo kldload ./sonic.ko && sudo kldload ./pencake.ko && sudo dmesg | grep "pnso_init failed with rc = 35" && sudo kldunload pencake && sudo kldunload sonic
errcode=$?
echo "errcode is $errcode"
exit $errcode
