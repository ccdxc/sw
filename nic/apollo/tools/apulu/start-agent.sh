#!/bin/sh

CUR_DIR=$( readlink -f $( dirname $0 ) )
source $CUR_DIR/setup_env_hw.sh

# remove logs
rm -f $NON_PERSISTENT_LOGDIR/pds-agent.log*

ulimit -c unlimited

exec $PDSPKG_TOPDIR/bin/pdsagent -c hal_hw.json $*

