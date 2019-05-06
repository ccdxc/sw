#!/bin/bash

if [[ -z "$GOPATH" ]]; then
    echo "GOPATH: $GOPATH is not defined"
    exit 1
fi

pkill iota_server
cd $GOPATH/src/github.com/pensando/sw/iota
if [ -f /warmd.json ]; then
    echo Starting IOTA server with the following contents of warmd.json
    cat /warmd.json
fi
VENICE_DEV=1 bin/server/iota_server > server.log 2>&1 &
echo "Started IOTA Server: in " $PWD " with log file server.log"
