#! /bin/bash -e
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`
SNORT_EXPORT_DIR=$NIC_DIR/hal/third-party/snort3/export
#GDB='gdb --args'
export ZMQ_SOC_DIR=$NIC_DIR
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NIC_DIR/../bazel-bin/nic/model_sim/:$SNORT_EXPORT_DIR/x86_64/lib/:$NIC_DIR/sdk/obj/lib:$NIC_DIR/conf/linkmgr:$NIC_DIR/sdk/obj/lib/external:$NIC_DIR/hal/obj
export SNORT_LUA_PATH=$SNORT_EXPORT_DIR/lua/
export LUA_PATH="$SNORT_EXPORT_DIR/lua/?.lua;;"
export SNORT_DAQ_PATH=$SNORT_EXPORT_DIR/x86_64/lib/

echo "STARTING fte_sim: `date +%x_%H:%M:%S:%N`"
/go/bin/tmagent -resolver-urls "192.168.30.10:9009" &
$NIC_DIR/../bazel-bin/nic/utils/agent_api/fte_sim/fte_sim  2>&1
