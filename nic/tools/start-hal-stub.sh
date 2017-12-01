#! /bin/bash
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`

export HAL_CONFIG_PATH=$NIC_DIR/conf/
#export HAL_PLUGIN_PATH=$NIC_DIR/../bazel-bin/nic/hal/plugins/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NIC_DIR/../bazel-bin/nic/model_sim/
export SNORT_LUA_PATH=$NIC_DIR/third-party/snort3/export/lua/
export LUA_PATH="$NIC_DIR/third-party/snort3/export/lua/?.lua;;"
export SNORT_DAQ_PATH=$NIC_DIR/third-party/snort3/export/daqs/
export SNORT_LD_PATH=$NIC_DIR/third-party/snort3/export/bin/libsnort.so

LD_PRELOAD=$SNORT_LD_PATH CAPRI_MOCK_MODE=1 $NIC_DIR/../bazel-bin/nic/hal/hal -c hal.json 2>&1 | tee $NIC_DIR/hal.log
