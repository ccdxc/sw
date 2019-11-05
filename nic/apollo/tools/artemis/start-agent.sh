#!/bin/sh

# remove logs
rm -f $LOG_DIR/pen-agent.log*

ulimit -c unlimited

export LD_LIBRARY_PATH=$LIBRARY_PATH
exec $NIC_DIR/bin/pdsagent -c hal_hw.json -f artemis $*
