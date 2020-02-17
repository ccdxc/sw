#! /bin/bash -e

CUR_DIR=$( readlink -f $( dirname $0 ) )
source $CUR_DIR/../setup_env_mock.sh apulu

# Enable the below to test with flow insertion
export AGENT_TEST_HOOKS_LIB=libflowtestagenthooks.so

echo "Starting Agent: `date +%x_%H:%M:%S:%N`"
rm -f $PDSPKG_TOPDIR/conf/pipeline.json
ln -s $PDSPKG_TOPDIR/conf/apulu/pipeline.json $PDSPKG_TOPDIR/conf/pipeline.json
$GDB $BUILD_DIR/bin/pdsagent -c hal.json $* 2>&1
#$VALGRIND $BUILD_DIR/bin/pdsagent -c hal.json $* 2>&1
