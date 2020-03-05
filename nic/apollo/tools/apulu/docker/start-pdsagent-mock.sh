#! /bin/bash -e

CUR_DIR=$( readlink -f $( dirname $0 ) )
source $CUR_DIR/setup_env_mock.sh apulu

echo "Starting PDS Agent: `date +%x_%H:%M:%S:%N`"
$PDSPKG_TOPDIR/bin/pdsagent -c hal.json $* 2>&1
