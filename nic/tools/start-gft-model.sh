#!/bin/bash -e
export ASIC="${ASIC:-capri}"
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`
export ZMQ_SOC_DIR=${NIC_DIR}
GFT_BLD_DIR=$NIC_DIR/build/x86_64/gft/${ASIC}
$GDB $GFT_BLD_DIR/bin/cap_model +PLOG_MAX_QUIT_COUNT=0 +plog=info \
    +model_debug=$GFT_BLD_DIR/gen/p4gen/gft/dbg_out//model_debug.json 2>&1 | tee $NIC_DIR/model.log
