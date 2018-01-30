#! /bin/bash -e
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`

export ZMQ_SOC_DIR=$NIC_DIR
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export HAL_PLUGIN_PATH=$NIC_DIR/../bazel-bin/nic/hal/plugins/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NIC_DIR/../bazel-bin/nic/model_sim/:$SNORT_EXPORT_DIR/bin/
# $GDB $NIC_DIR/../bazel-bin/nic/linkmgr/linkmgr 2>&1 | tee $NIC_DIR/linkmgr.log
$GDB $NIC_DIR/../bazel-bin/nic/linkmgr/linkmgr
