#!/bin/sh

LOGFILE=/obfl/bflog
LOGMAXSZ=1048576

[ ! -f /nic/bin/bflog ] && exit 0

if [ -f $LOGFILE ]; then
    sz=`stat -c "%s" $LOGFILE`
    if [ $sz -gt $LOGMAXSZ ]; then
        [ -f ${LOGFILE}.1 ] && mv ${LOGFILE}.1 ${LOGFILE}.2
        mv $LOGFILE ${LOGFILE}.1
        : > $LOGFILE
    fi
fi

n=`/nic/bin/bflog tail -r 2>&1 | wc -l`
if [ $n -gt 0 ]; then
    date >> $LOGFILE
    /nic/bin/bflog tail -xm >> $LOGFILE 2>&1
fi
