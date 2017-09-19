#! /bin/bash
if [ -z "$NIC_DIR" ]; then
    export NIC_DIR=`dirname $PWD`
fi
if [ `basename $NIC_DIR` != "nic" ]; then
    echo "Run $0 from tools dir or set env NIC_DIR"
    exit 1
fi
export HAL_CONFIG_PATH=$NIC_DIR/conf/
CAPRI_MOCK_MODE=1 LD_LIBRARY_PATH=$NIC_DIR/obj:$NIC_DIR/asic/capri/model/capsim-gen/lib $NIC_DIR/obj/hal_stub -c hal.json 2>&1 | tee $NIC_DIR/hal.log
