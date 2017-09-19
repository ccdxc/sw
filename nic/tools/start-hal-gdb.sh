#! /bin/bash
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`
export HAL_CONFIG_PATH=$NIC_DIR/conf/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NIC_DIR/obj gdb --args $NIC_DIR/obj/hal -c hal.json 2>&1 | tee $NIC_DIR/hal-gdb.log
