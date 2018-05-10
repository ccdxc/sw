#!/bin/bash 
if [ "$NAPLES_DIR" = "" ]; then
    export NAPLES_DIR="/var/naples"
fi

#NAT Gateway IP and nat subnet 
NAT_GWIP=10.101.0.1      # IP address of the NAT gateway 
NAT_SUBNET=10.100.0.0/16 # This should match the nat IP pool
LOCAL_SUBNET_VLAN=100

#naples uplink interfaces
INTF1=pen-intf1
INTF2=pen-intf2

#naples host interface
HOST0=pen-host0

LOGDIR=${NAPLES_DIR}/logs

########################################################################
# Setup intf1 (redirect all the traffic detined to NAT_SUBNET via intf1)
########################################################################
function setup_intf1()
{
    while true; do
        sleep 5
        pid=$(docker inspect --format '{{.State.Pid}}' naples-sim)
        nsenter -t $pid -n ip link set $INTF1 netns 1 >& /dev/null
        if [ $? -eq 0 ]; then
            set -x
            ip link set up dev $INTF1
            subif=$INTF1.$LOCAL_SUBNET_VLAN

            ip link add link $INTF1 name $subif type vlan id $LOCAL_SUBNET_VLAN
            ip link set up dev $subif

            #set ip/mac address
            ip link set dev $subif address 00:22:22:22:22:23
            ip addr add dev $subif 10.100.255.255/16

            #interface route for NAT gateway IP
            ip route add $NAT_GWIP/32 dev $subif
            
            #static ARP for NAT gateway IP
            ip neigh add 10.101.0.1 lladdr 00:0c:ba:ba:ba:ba dev $subif
            
            #static route for NAT subnet
            ip route add $NAT_SUBNET via $NAT_GWIP
            set +x
        fi
    done
}

#######################################################################
#Setup intf2 (connect it to the other node via linux bridge)
#######################################################################
function setup_intf2()
{
    while true; do
        sleep 5
        pid=$(docker inspect --format '{{.State.Pid}}' naples-sim)
        nsenter -t $pid -n ip link set $INTF2 netns 1  >& /dev/null
        if [ $? -eq 0 ]; then
            set -x
            brctl addif br0 $INTF2
            ip link set up dev $INTF2
            set +x
        fi
    done
}

# redirect output to bootstrap.log
mkdir -p $LOGDIR
exec > $LOGDIR/bootstrap.log
exec 2>&1
set -x

DEFAULT_INTF=`sudo route | grep default | tr -s \   | cut -d \  -f 8`
sudo yum install -y bridge-utils tcpdump net-tools
#setup the bridge
sudo brctl addbr br0
sudo brctl stp br0 off
sudo brctl addif br0 ${DEFAULT_INTF}
sudo ip link set up dev br0
sudo ip link set up dev ${DEFAULT_INTF}
sudo dhclient br0
sudo ip addr flush dev ${DEFAULT_INTF}

# setup the uplinks in background
$(setup_intf1 >& $LOGDIR/bootstrap-intf1.log) &
$(setup_intf2 >& $LOGDIR/bootstrap-intf2.log) &

