#!/bin/bash -e

TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`
export ZMQ_SOC_DIR=$NIC_DIR
BLD_DIR=$NIC_DIR/build/x86_64/apollo
/sw/nic/tools/merge_model_debug.py --p4 apollo --rxdma apollo_rxdma --txdma apollo_txdma
echo "STARTING MODEL: `date +%x_%H:%M:%S:%N`"  
$GDB $BLD_DIR/bin/cap_model +PLOG_MAX_QUIT_COUNT=0 +plog=info +plog_add_scope=axi_trace +model_debug=$BLD_DIR/gen/p4gen/apollo/dbg_out/combined_model_debug.json 2>&1 | tee $NIC_DIR/model.log
