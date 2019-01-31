#!/bin/sh

export NIC_DIR='/nic'
export PLATFORM_DIR='/platform'
export LD_LIBRARY_PATH=$NIC_DIR/lib:$PLATFORM_DIR/lib
ulimit -c unlimited
cd /
# start pciemgrd
/nic/bin/pciemgrd-gold &
# start memtun
/platform/bin/memtun &

echo "Launched all applications ..."
