cd `dirname $0`       
if [ -z "$NIC_DIR" ]; then
    export NIC_DIR=`dirname $PWD`
fi
export LD_LIBRARY_PATH=$NIC_DIR/model_sim/:$NIC_DIR/model_sim/libs:/home/asic/tools/src/0.25/x86_64/lib64:$LD_LIBRARY_PATH        
gdb --args $NIC_DIR/model_sim/build/cap_model +plog=info +model_debug=$NIC_DIR/gen/iris/dbg_out/model_debug.json | tee $NIC_DIR/model_gdb.log
