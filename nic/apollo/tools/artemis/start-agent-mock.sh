#! /bin/bash -e

export ASIC="${ASIC:-capri}"
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR/../../../../`
#GDB='gdb --args'
# Enable the below to test with flow insertion
export AGENT_TEST_HOOKS_LIB=libflowtestagenthooks.so
export CONFIG_PATH=$NIC_DIR/conf/
export LOG_DIR=$NIC_DIR/
export PERSISTENT_LOG_DIR=$NIC_DIR/
export ASIC_MOCK_MODE=1
export ASIC_MOCK_MEMORY_MODE=1
export IPC_MOCK_MODE=1
export ZMQ_SOC_DIR=${NIC_DIR}
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NIC_DIR/third-party/metaswitch/output/x86_64/
echo "Starting Agent: `date +%x_%H:%M:%S:%N`"
BUILD_DIR=$NIC_DIR/build/x86_64/artemis/${ASIC}
export COVFILE=$NIC_DIR/coverage/sim_bullseye_hal.cov
rm -f $NIC_DIR/conf/pipeline.json
ln -s $NIC_DIR/conf/artemis/pipeline.json $NIC_DIR/conf/pipeline.json
$GDB $BUILD_DIR/bin/pdsagent -c hal.json $* 2>&1
#valgrind --leak-check=full --show-leak-kinds=all --gen-suppressions=all --error-limit=no --verbose --log-file=valgrind-out.txt --track-origins=yes $BUILD_DIR/bin/pdsagent -c hal.json $* 2>&1
