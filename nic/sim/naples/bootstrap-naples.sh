#!/bin/bash 

#naples uplink interfaces
INTF1=pen-intf1
INTF2=pen-intf2

#naples IF count
NUM_IF=16
LOGDIR=/var/run/naples/logs

LONGOPTIONS="qemu,naples-sim-name:"

PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTIONS --name "$0" -- "$@")
if [[ $? -ne 0 ]]; then
    # e.g. $? == 1
    #  then getopt has complained about wrong arguments to stdout
    exit 2
fi

eval set -- "$PARSED"
qemu=0
naples_sim_name="naples-sim"
while true; do
    case "$1" in
         --qemu)
            qemu=1
            shift
            ;;
         --naples-sim-name)
            naples_sim_name=$2
            shift 2
            break
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "Programming error"
            exit 3
            ;;
    esac
done

#######################################################################
#Setup intf1 (connect it to the other node via linux bridge)
#######################################################################
function setup_intf1()
{
    set +x
    while true; do
        sleep 5
        pid=$(docker inspect --format '{{.State.Pid}}' $naples_sim_name)
        nsenter -t $pid -n ip link set $INTF1 netns 1  >& /dev/null
        if [ $? -eq 0 ]; then
            set -x
            #ovs-vsctl add-port data-net pen-intf1
            ip link set up dev $INTF1
            ip link set mtu 9216 dev $INTF1
            set +x
        fi
    done
}

#######################################################################
#Setup intf2 (connect it to the other node via linux bridge)
#######################################################################
function setup_intf2()
{
    set +x
    while true; do
        sleep 5
        pid=$(docker inspect --format '{{.State.Pid}}' $naples_sim_name)
        nsenter -t $pid -n ip link set $INTF2 netns 1  >& /dev/null
        if [ $? -eq 0 ]; then
            set -x
            #ovs-vsctl add-port data-net pen-intf2
            ip link set up dev $INTF2
            ip link set mtu 9216 dev $INTF2
            set +x
        fi
    done
}

# redirect output to bootstrap.log
mkdir -p $LOGDIR
exec > $LOGDIR/bootstrap.log
exec 2>&1
set -x

if [ "$qemu" -eq 0 ]
then
    for ((intf=100; intf<100+$NUM_IF; intf++))
    do
        pid=$(docker inspect --format '{{.State.Pid}}' $naples_sim_name)
        nsenter -t $pid -n ip link set lif$intf netns 1 >& /dev/null
        while [ $? -ne 0 ]; do
            sleep 5
            nsenter -t $pid -n ip link set lif$intf netns 1 >& /dev/null
        done
        ip link set up dev lif$intf
    done
fi


# setup the uplinks in background
$(setup_intf1  >& $LOGDIR/bootstrap-intf1.log) &
$(setup_intf2  >& $LOGDIR/bootstrap-intf2.log) &
