export ASIC="${ASIC:-capri}"
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR`/../../
SNORT_EXPORT_DIR=$NIC_DIR/hal/third-party/snort3/export
GDB='gdb --args'
export ZMQ_SOC_DIR=$NIC_DIR
export CONFIG_PATH=$NIC_DIR/conf/
export SNORT_LUA_PATH=$SNORT_EXPORT_DIR/lua/
export LUA_PATH="$SNORT_EXPORT_DIR/lua/?.lua;;"
export SNORT_DAQ_PATH=$SNORT_EXPORT_DIR/x86_64/lib/
export GRPC_TRACE=api

echo "STARTING APOLLO: `date +%x_%H:%M:%S:%N`"
rm -f $NIC_DIR/conf/pipeline.json
ln -s $NIC_DIR/conf/apollo/pipeline.json $NIC_DIR/conf/pipeline.json
$GDB $NIC_DIR/build/x86_64/apollo/${ASIC}/bin/apollo_test
rm -f $NIC_DIR/conf/pipeline.json
