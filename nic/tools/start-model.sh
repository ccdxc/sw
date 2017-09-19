#!/bin/bash -e

cd `dirname $0`        
if [ -z "$NIC_DIR" ]; then
    NIC_DIR=`dirname $PWD`      
fi
if [ `basename $NIC_DIR` != "nic" ]; then
    echo "Run $0 from tools dir or set env NIC_DIR"
    exit 1
fi
export LD_LIBRARY_PATH=$NIC_DIR/model_sim/:$NIC_DIR/model_sim/libs:/home/asic/tools/src/0.25/x86_64/lib64:$LD_LIBRARY_PATH        
$GDB $NIC_DIR/model_sim/build/cap_model +plog=info +model_debug=$NIC_DIR/gen/iris/dbg_out/model_debug.json 2>&1 | tee $NIC_DIR/model.log
