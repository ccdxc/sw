#!/bin/sh

CMDS=("hardware" "interface" "error" "trace" "logging")
LOGFILE=$LOG_DIR/vpp.log

# TODO: move to techsupport & remove this file
function collect_vpp_logs () {
    for cmd in ${CMDS[@]}; do
        $PDSPKG_TOPDIR/vpp/tools/vppctl-sim.sh show $cmd >> $LOGFILE 2>&1
    done
}
collect_vpp_logs
rm -f $CONFIG_PATH/vpp_startup.conf

