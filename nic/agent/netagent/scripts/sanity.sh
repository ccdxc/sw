#!/bin/bash

function cleanup {
  pkill cap_model
  pkill hal
  $GOPATH/src/github.com/pensando/sw/nic/tools/savelogs.sh
  exit $1

}
set -e
./tools/start-model.sh > /dev/null &
./tools/start-hal.sh > /dev/null &
./agent/netagent/scripts/wait-for-hal.sh || cleanup $?
./build/x86_64/iris/bin/fake_nic_mgr || cleanup $?

cd $GOPATH/src/github.com/pensando/sw
if [ "$1" == "single-threaded" ]; then
  export GOMAXPROCS=1
fi
if [ "$1" == "stand-alone" ]; then
  export E2E_AGENT_DATAPATH=HAL
  go test -v ./nic/agent/tests/standalone || cleanup $?
  cleanup
fi
go test -v ./test/integ/venice_integ -run TestVeniceInteg -datapath=hal -agents=1 || cleanup $?
go test -v ./test/integ/npminteg/... || cleanup $?
cleanup
