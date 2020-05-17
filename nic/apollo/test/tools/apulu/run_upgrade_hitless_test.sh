#! /bin/bash

set -x

MY_DIR=$( readlink -f $( dirname $0 ))
export DOL_ARGS='--pipeline apulu --topo learn --feature learn'
export PIPELINE=apulu
source $MY_DIR/../../../tools/setup_env_sim.sh $PIPELINE
source $PDSPKG_TOPDIR/sdk/upgrade/core/upgmgr_core_base.sh

# clear any upgrade specific data from previous failed run
rm -rf /update/*       # upgrade init mode
rm -rf /root/.pcie*  # pciemgrd saves here in sim mode
rm -rf /sw/nic/*.log /sw/nic/core.*

function kill_process() {
    pid=`pgrep -f $1`
    if [ -n "$pid" ];then
        kill -TERM $pid
        sleep $2
    fi
}

function trap_finish () {
    kill_process 'start_upgrade_hitless_process.sh' 5
    kill_process 'cap_model' 0
    kill_process 'main.py' 0
    kill_process 'dhcpd' 0
}
trap trap_finish EXIT SIGTERM

# start the model
# model used the current directory in sim. this cannot be avoided
$MY_DIR/start-model.sh &
sudo $MY_DIR/../../../tools/$PIPELINE/start-dhcpd-sim.sh -p apulu

# set the upgrade domain id
rm -f  /tmp/agent_up
# set the domain
sh $MY_DIR/start_upgrade_hitless_process.sh $UPGRADE_DOMAIN_A &
# wait for this to be up
count=0
while [ ! -f /tmp/agent_up ];do
    echo "waiting for agent up"
    sleep 1
    count=`expr $count + 1`
    if [ $count -ge 240 ];then
        echo "Agent bringup time exceeded"
        exit 1
    fi
done

# start DOL now
# dol required the below for device config and log access
export PDSPKG_TOPDIR="/tmp/$UPGRADE_DOMAIN_A"
# comment out below for debugging any application where config
# is not needed
#/sw/dol/main.py $DOL_ARGS 2>&1 | tee dol.log
#status=${PIPESTATUS[0]}

$BUILD_DIR/bin/pdsupgclient -m hitless
