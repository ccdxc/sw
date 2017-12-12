#! /bin/bash -e
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`

export HAL_CONFIG_PATH=$NIC_DIR/conf/
$GDB $NIC_DIR/../bazel-bin/nic/linkmgr/linkmgr 2>&1 | tee $NIC_DIR/linkmgr.log
