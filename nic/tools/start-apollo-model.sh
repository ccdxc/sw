#!/bin/bash -e
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`
export ZMQ_SOC_DIR=${NIC_DIR}
$GDB $NIC_DIR/build/x86_64/apollo/bin/cap_model \
    +PLOG_MAX_QUIT_COUNT=0 +plog=info \
    +model_debug=$NIC_DIR/gen/apollo/dbg_out/model_debug.json 2>&1 | tee $NIC_DIR/model.log
