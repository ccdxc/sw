#!/bin/bash -e
export ASIC="${ASIC:-capri}"
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`
export ZMQ_SOC_DIR=$NIC_DIR
IRIS_BLD_DIR=$NIC_DIR/build/x86_64/iris/${ASIC}
gdb --args $NIC_DIR/build/x86_64/iris/${ASIC}/bin/cap_model +PLOG_MAX_QUIT_COUNT=0 +plog=info +model_debug=$IRIS_BLD_DIR/gen/p4gen/p4/dbg_out//model_debug.json | tee $NIC_DIR/model.log
