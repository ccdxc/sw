#! /bin/bash -e

CUR_DIR=$( readlink -f $( dirname $0 ) )
source $CUR_DIR/../setup_env_sim.sh apulu

echo "Starting Agent: `date +%x_%H:%M:%S:%N`"
rm -f $PDSPKG_TOPDIR/conf/pipeline.json
ln -s $PDSPKG_TOPDIR/conf/apulu/pipeline.json $PDSPKG_TOPDIR/conf/pipeline.json
$GDB $BUILD_DIR/bin/pdsagent -c hal.json $* 2>&1
#$VALGRIND $BUILD_DIR/bin/pdsagent -c hal.json -f apulu $* 2>&1
