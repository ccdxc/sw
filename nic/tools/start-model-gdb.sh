#!/bin/bash -e
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`
gdb --args $NIC_DIR/../bazel-bin/nic/model_sim/cap_model +PLOG_MAX_QUIT_COUNT=0 +plog=info +model_debug=$NIC_DIR/gen/iris/dbg_out/model_debug.json | tee $NIC_DIR/model.log
