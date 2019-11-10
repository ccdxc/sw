#!/bin/sh

# remove logs
rm -f $LOG_DIR/pds-agent.log*

ulimit -c unlimited

export PERSISTENT_LOG_DIR=/obfl/
export LD_LIBRARY_PATH=$LIBRARY_PATH
exec $NIC_DIR/bin/pdsagent -c hal_hw.json -f apollo $*
