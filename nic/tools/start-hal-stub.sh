#! /bin/bash
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`

export ZMQ_SOC_DIR=$NIC_DIR
export HAL_CONFIG_PATH=$NIC_DIR/conf/
#export HAL_PLUGIN_PATH=$NIC_DIR/conf/plugins/

ASIC_MOCK_MODE=1 $NIC_DIR/build/x86_64/iris/bin/hal -c hal.json 2>&1
