#! /bin/bash -e
if [ -z "$NIC_DIR" ]; then
    export NIC_DIR=`dirname $PWD`
fi
export HAL_CONFIG_PATH=$NIC_DIR/conf/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NIC_DIR/obj:$NIC_DIR/asic/capri/model/capsim-gen/lib $GDB $NIC_DIR/obj/hal -c hal.json 2>&1 | tee $NIC_DIR/hal.log
