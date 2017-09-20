#! /bin/bash -e
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`

export HAL_CONFIG_PATH=$NIC_DIR/conf/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NIC_DIR/obj:$NIC_DIR/asic/capri/model/capsim-gen/lib:$NIC_DIR/third-party/lkl/export/bin $GDB $NIC_DIR/obj/hal -c hal.json 2>&1 | tee $NIC_DIR/hal.log
