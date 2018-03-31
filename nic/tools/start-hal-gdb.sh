#! /bin/bash -e
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`
SNORT_EXPORT_DIR=$NIC_DIR/hal/third-party/snort3/export
export ZMQ_SOC_DIR=$NIC_DIR
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NIC_DIR/../bazel-bin/nic/model_sim/:$SNORT_EXPORT_DIR/x86_64/lib/:$NIC_DIR/sdk/obj/lib:$NIC_DIR/sdk/obj/lib/external/
export SNORT_LUA_PATH=$SNORT_EXPORT_DIR/lua/
export LUA_PATH="$SNORT_EXPORT_DIR/lua/?.lua;;"
export SNORT_DAQ_PATH=$SNORT_EXPORT_DIR/x86_64/lib/

#gdb --args $NIC_DIR/../bazel-bin/nic/hal/hal -c hal.json | tee $NIC_DIR/hal_gdb.log
gdb --args $NIC_DIR/../bazel-bin/nic/hal/hal -c hal.json
