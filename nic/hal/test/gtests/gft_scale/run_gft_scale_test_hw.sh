#! /bin/bash

set -e
export NIC_DIR=/nic/
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export NON_PERSISTENT_LOG_DIR=/var/log/pensando/
export HAL_LIBRARY_PATH=$NIC_DIR/lib:$NIC_DIR/../platform/lib:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
export HAL_PBC_INIT_CONFIG="2x100_hbm"
export HAL_PLATFORM_HW=1
export DISABLE_FTE=1
#GDB='gdb --args'
$GDB $NIC_DIR/bin/gft_scale_test -c hal_hw.json &> /data/std.txt &
