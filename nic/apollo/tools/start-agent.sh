#!/bin/sh

SYSCONFIG=/sysconfig/config0
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
export NIC_DIR=`dirname $ABS_TOOLS_DIR`
export CONFIG_PATH=$NIC_DIR/conf/
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LOG_DIR=/var/log/pensando/
export LIBRARY_PATH=$NIC_DIR/lib:$NIC_DIR/../platform/lib:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
export HAL_PBC_INIT_CONFIG="2x100_hbm"

# remove logs
rm -f $LOG_DIR/pen-agent.log*

ulimit -c unlimited

export LD_LIBRARY_PATH=$LIBRARY_PATH
exec $NIC_DIR/bin/agent -c hal_hw.json

[[ $? -ne 0 ]] && echo "Failed to start agent!" && exit 1
