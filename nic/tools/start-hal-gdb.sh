#! /bin/bash
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`
export HAL_CONFIG_PATH=$NIC_DIR/conf/
gdb --args $NIC_DIR/../bazel-bin/nic/hal/hal -c hal.json | tee $NIC_DIR/hal_gdb.log
