#!/bin/bash

function cleanup {
  pkill cap_model
  pkill hal
  exit $1

}
set -e
if [ "$1" == "single-threaded" ]; then
  export GOMAXPROCS=1
fi

./tools/start-model.sh > /dev/null &
./tools/start-hal.sh > /dev/null &
./agent/netagent/scripts/wait-for-hal.sh || cleanup $?
cd $GOPATH/src/github.com/pensando/sw
go test -v ./test/integ/venice_integ -run TestVeniceInteg -datapath=hal -agents=1 || cleanup $?
go test -v ./test/integ/npminteg/... || cleanup $?
cleanup
