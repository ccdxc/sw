#!/bin/bash
set -x

if [[ -z "$GOPATH" ]]; then
    echo "GOPATH: $GOPATH is not defined"
    exit 1
fi

WARMD_FILE=/warmd.json
if [[ $# -ge 1 ]] ;
then
    WARMD_FILE=$1
fi

pkill iota_server
cd $GOPATH/src/github.com/pensando/sw/iota
if [ -f ${WARMD_FILE} ]; then
    echo Starting IOTA server with the following contents of warmd.json
    cat ${WARMD_FILE}
fi

VENICE_DEV=1 nohup $GOPATH/src/github.com/pensando/sw/iota/bin/server/iota_server > server.log 2>&1 &
echo "Started IOTA Server: in " $PWD " with log file server.log"
