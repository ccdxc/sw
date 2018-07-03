#!/bin/bash 

#NAT Gateway IP and nat subnet 
NAT_GWIP=10.101.0.1      # IP address of the NAT gateway 
NAT_SUBNET=10.100.0.0/16 # This should match the nat IP pool
LOCAL_SUBNET_VLAN=100

#NAT over Overlay
NAT_OVLY_GWIP=10.201.0.1  # IP address of the NAT gateway
NAT_OVLY_SUBNET=10.200.0.0/16 # This should match the nat pool

#HOST_SUBNET
HOST_SUBNET=20.30.10.0/24 #ip address of l2seg network
HOST_VLAN=200

HOST1_SUBNET=10.30.0.0/16 #ip address of second l2seg network
HOST1_VLAN=300

#naples uplink interfaces
INTF1=pen-intf1
INTF2=pen-intf2

#naples host interface
HOST0=lif103
HOST1=lif106


#naples IF count
NUM_IF=16

LOGDIR=/var/run/naples/logs

########################################################################
# Setup intf1 (redirect all the traffic detined to NAT_SUBNET via intf1)
########################################################################
function setup_intf1()
{
    set +x
    while true; do
        sleep 5
        pid=$(docker inspect --format '{{.State.Pid}}' naples-sim)
        nsenter -t $pid -n ip link set $INTF1 netns 1 >& /dev/null
        if [ $? -eq 0 ]; then
            set -x
            ip link set up dev $INTF1
            #ip link set mtu 9216 dev $INTF1
            subif=$INTF1.$LOCAL_SUBNET_VLAN

            ip link add link $INTF1 name $subif type vlan id $LOCAL_SUBNET_VLAN
            ip link set up dev $subif
            #ip link set mtu 9216 dev $subif

            #set ip/mac address
            ip link set dev $subif address 00:22:0a:00:02:01
            ip addr add dev $subif 10.0.2.1/32

            #interface route for NAT gateway IP
            ip route add $NAT_GWIP/32 dev $subif
            
            #static ARP for NAT gateway IP
            ip neigh add 10.101.0.1 lladdr 00:22:0a:65:00:01 dev $subif
            
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
    set +x
    while true; do
        sleep 5
        pid=$(docker inspect --format '{{.State.Pid}}' naples-sim)
        nsenter -t $pid -n ip link set $INTF2 netns 1  >& /dev/null
        if [ $? -eq 0 ]; then
            set -x
            ovs-vsctl add-port data-net pen-intf2
            ip link set up dev $INTF2
            ip link set mtu 9216 dev $INTF2
            set +x
        fi
    done
}

###############################################################################
# Setup intf3 ( HOST to HOST communication on the same l2seg over vxlan tunnel)
###############################################################################
function setup_intf3()
{
    set -x
    start=10
    local_host=`expr $start + $1`
    ip link set up dev $HOST0
    subif=$HOST0.$HOST_VLAN

    ip link add link $HOST0 name $subif type vlan id $HOST_VLAN
    ip link set up dev $subif

    #set ip/mac address
    ip link set dev $subif address 00:22:0a:00:03:$local_host
    ip link set dev $HOST0 address 00:22:0a:00:03:$local_host
    ip addr add dev $subif 20.30.10.$local_host/24

    #static ARP for remote ip
    if [ $1 -eq 1 ]; then
        remote_node=`expr $local_host + 1`
    else 
        remote_node=`expr $local_host - 1`
    fi
    ip neigh add 20.30.10.$remote_node lladdr 00:22:0a:00:03:$remote_node dev $subif
    
    set +x
}

#######################################################################################
# Setup intf4 (HOST to HOST communication with nat on the same l2seg over vxlan tunnel)
#######################################################################################
function setup_intf4()
{
    set -x
    start=10
    local_host=`expr $start + $1`
    ip link set up dev $HOST1
    subif=$HOST1.$HOST1_VLAN

    ip link add link $HOST1 name $subif type vlan id $HOST1_VLAN
    ip link set up dev $subif

    #static ARP for remote ip
    if [ $1 -eq 1 ]; then
        ip link set dev $subif address 00:22:0b:00:03:$local_host
        ip link set dev $HOST1 address 00:22:0b:00:03:$local_host
        ip addr add dev $subif 10.30.10.$local_host/16
        ip neigh add 10.30.0.64 lladdr 00:22:0b:00:02:15 dev $subif
    else 
        remote_node=`expr $local_host - 1`
        ip link set dev $subif address 00:22:0b:00:02:15
        ip link set dev $HOST1 address 00:22:0b:00:02:15
        ip addr add dev $subif 10.30.2.15/16
        ip neigh add 10.30.10.11 lladdr 00:22:0b:00:03:$remote_node dev $subif
    fi
    set +x
}


# redirect output to bootstrap.log
mkdir -p $LOGDIR
exec > $LOGDIR/bootstrap.log
exec 2>&1
set -x

for ((intf=100; intf<100+$NUM_IF; intf++))
do
    pid=$(docker inspect --format '{{.State.Pid}}' naples-sim)
    nsenter -t $pid -n ip link set lif$intf netns 1 >& /dev/null
    while [ $? -ne 0 ]; do
        sleep 5
        nsenter -t $pid -n ip link set lif$intf netns 1 >& /dev/null
    done
    ip link set up dev lif$intf
done
# setup the uplinks in background
$(setup_intf1 $1 >& $LOGDIR/bootstrap-intf1.log) &
$(setup_intf2 $1 >& $LOGDIR/bootstrap-intf2.log) &
$(setup_intf3 $1 >& $LOGDIR/bootstrap-intf3.log) &
$(setup_intf4 $1 >& $LOGDIR/bootstrap-intf4.log) &


