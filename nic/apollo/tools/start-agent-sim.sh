#! /bin/bash -e

TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR/../../../`
#GDB='gdb --args'
export CONFIG_PATH=$NIC_DIR/conf/
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export ZMQ_SOC_DIR=${NIC_DIR}
echo "Starting Agent: `date +%x_%H:%M:%S:%N`"
BUILD_DIR=$NIC_DIR/build/x86_64/apollo
$GDB $BUILD_DIR/bin/agent -c hal.json 2>&1
