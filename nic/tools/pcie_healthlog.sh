#!/bin/sh

LOGDIR=/obfl
LOGFILE=$LOGDIR/pciehealth.log
LOGMAXSZ=1048576

log_init()
{
    mkdir -p $LOGDIR
    sz=`stat -c "%s" $LOGFILE`
    if [ $sz -gt $LOGMAXSZ ]; then
        [ -f ${LOGFILE}.1 ] && mv ${LOGFILE}.1 ${LOGFILE}.2
        mv $LOGFILE ${LOGFILE}.1
    fi
    exec >>$LOGFILE 2>&1
}

port=${1-0}
reason=${2-""}
log_init
echo "================"
echo "`date '+[%Y%m%d-%H:%M:%S]'` begin port$port healthlog $reason"
echo "================"

pcieutil=/platform/bin/pcieutil
cmds="
    $pcieutil healthdump -p $port
    $pcieutil linkpoll -p $port -a -T 5
"

echo "$cmds" | \
while read cmd; do
    if [ -z "$cmd" ]; then continue; fi
    echo "================"
    echo "$cmd"
    echo "----------------"
    $cmd
done
sync
