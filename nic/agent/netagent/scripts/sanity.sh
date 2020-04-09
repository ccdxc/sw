#!/bin/bash

export ASIC="${ASIC:-capri}"
function cleanup {
  pkill cap_model
  pkill hal
  $GOPATH/src/github.com/pensando/sw/nic/tools/savelogs.sh
  exit $1

}
set -e
# This is needed only for integ tests with HAL. In other real e2e environment
# it is set by start-hal-haps.sh by reading /mnt partition which has the primary mac for naples
export SYSUUID="42:42:42:42:42:42"
./tools/start-model.sh > /dev/null &
./tools/start-hal.sh > /dev/null &
./agent/netagent/scripts/wait-for-hal.sh || cleanup $?
./build/x86_64/iris/${ASIC}/bin/fake_nic_mgr || cleanup $?

export VENICE_DEV=1
cd $GOPATH/src/github.com/pensando/sw
if [ "$1" == "single-threaded" ]; then
  export GOMAXPROCS=1
fi
if [ "$1" == "stand-alone" ]; then
  export E2E_AGENT_DATAPATH=HAL
  LD_LIBRARY_PATH=$GOPATH/src/github.com/pensando/sw/nic/build/x86_64/iris/${ASIC}/lib/ go test -v ./nic/agent/tests/standalone || cleanup $?
  cleanup
fi
go test -v ./test/integ/venice_integ -run TestVeniceInteg -datapath=hal -hosts=1 -timeout=15m || cleanup $?
go test -v ./test/integ/npminteg/... || cleanup $?
cleanup
