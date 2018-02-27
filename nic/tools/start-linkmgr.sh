#! /bin/bash -e
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`

export ZMQ_SOC_DIR=$NIC_DIR
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NIC_DIR/../bazel-bin/nic/model_sim/:$SNORT_EXPORT_DIR/x86_64/lib/:$HAL_CONFIG_PATH/libs/sdk/
# $GDB $NIC_DIR/../bazel-bin/nic/linkmgr/linkmgr 2>&1 | tee $NIC_DIR/linkmgr.log
$GDB $NIC_DIR/../bazel-bin/nic/linkmgr/linkmgr
