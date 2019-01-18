#!/bin/sh
cd /tmp/git/pensando/sw/storage/offload && OS_DIR=/usr/src/freebsd ./freebsd_build.sh && sudo kldload ./sonic.ko && sudo kldload ./pencake.ko && dmesg | grep "pnso_init failed with rc = 35" 
