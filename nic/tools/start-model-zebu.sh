#!/bin/bash -e
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`
export ZMQ_SOC_DIR=$NIC_DIR
export LD_LIBRARY_PATH=$NIC_DIR/lib:/usr/local/lib:$LD_LIBRARY_PATH
export MODEL_ZMQ_TYPE_TCP=1
ulimit -c unlimited
ldd $NIC_DIR/bin/cap_model
echo "STARTING MODEL: `date +%x_%H:%M:%S:%N`"  
$GDB $NIC_DIR/bin/cap_model +PLOG_MAX_QUIT_COUNT=0 +plog=info +model_debug=$NIC_DIR/conf/model_debug.json 2>&1 | tee model.log
