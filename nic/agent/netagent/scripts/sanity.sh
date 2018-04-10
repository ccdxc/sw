#!/bin/bash

function cleanup {
  pkill cap_model
  pkill hal
  exit $1

}
set -e
./tools/start-model.sh > /dev/null &
./tools/start-hal.sh > /dev/null &
./agent/netagent/scripts/wait-for-hal.sh || cleanup $?
cd $GOPATH/src/github.com/pensando/sw
if [ "$1" == "single-threaded" ]; then
  export GOMAXPROCS=1
fi

bazel run //nic/hal/test:fake_nic_mgr || cleanup $?
if [ "$1" == "stand-alone" ]; then
  export E2E_AGENT_DATAPATH=HAL
  go test -v ./nic/agent/tests/standalone || cleanup $?
  cleanup
fi
go test -v ./test/integ/venice_integ -run TestVeniceInteg -datapath=hal -agents=1 || cleanup $?
go test -v ./test/integ/npminteg/... || cleanup $?
cleanup
