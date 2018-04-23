#! /bin/bash
export PATH=$GOPATH/bin:$PATH
cd $GOPATH/src/github.com/pensando/sw/
rm /tmp/naples-netagent.db
go install github.com/pensando/sw/nic/agent/cmd/netagent
bash nic/agent/netagent/scripts/wait-for-hal.sh
netagent -hostif lo -logtofile /sw/nic/agent.log -datapath hal
