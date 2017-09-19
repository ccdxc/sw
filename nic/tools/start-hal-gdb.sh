#! /bin/bash
if [ -z "$NIC_DIR" ]; then
    export NIC_DIR=`dirname $PWD`
fi
if [ `basename $NIC_DIR` != "nic" ]; then
    echo "Run $0 from tools dir or set env NIC_DIR"
    exit 1
fi
export HAL_CONFIG_PATH=$NIC_DIR/conf/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NIC_DIR/obj gdb --args $NIC_DIR/obj/hal -c hal.json 2>&1 | tee $NIC_DIR/hal-gdb.log
