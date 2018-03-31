#! /bin/bash -e
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`

export ZMQ_SOC_DIR=$NIC_DIR
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NIC_DIR/../bazel-bin/nic/model_sim/:$SNORT_EXPORT_DIR/x86_64/lib/:$NIC_DIR/sdk/obj/lib:$NIC_DIR/sdk/obj/lib/external/
rm -f $NIC_DIR/linkmgr.log
$GDB $NIC_DIR/../bazel-bin/nic/linkmgr/linkmgr
