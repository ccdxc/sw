#!/bin/bash 

#NATR Gateway IP and nat subnet 
NAT_GWIP=10.101.0.1      # IP address of the NAT gateway 
NAT_SUBNET=10.100.0.0/16 # This should match the nat IP pool

#naples uplink interfaces
INTF1=pen-intf1
INTF2=pen-intf2

#naples host interface
HOST0=pen-host0

# redirect output to bootstrap.log
exec > /var/log/bootstrap.log
exec 2>&1

set -x

#######################################################################
# move the tap interfaces inside the naples container to global namespace
#######################################################################
pid=$(docker inspect --format '{{.State.Pid}}' naples-sim)

nsenter -t $pid -n ip link show dev $INTF1
while [ $? -ne 0 ]; do
    sleep 5
    nsenter -t $pid -n ip link show dev $INTF1
done

#nsenter -t $pid -n ip link set $HOST0 netns 1
nsenter -t $pid -n ip link set $INTF1 netns 1
nsenter -t $pid -n ip link set $INTF2 netns 1

#######################################################################
# Setup intf1 (redirect all the traffic detined to NAT_SUBNET via intf1)
#######################################################################
ip link set up dev $INTF1

#interface route for NAT gateway IP
ip route add $NAT_GWIP/32 dev $INTF1

#static ARP for NAT gateway IP
ip neigh add 10.101.0.1 lladdr 00:0c:ba:ba:ba:ba dev pen-intf1

#static route for NAT subnet
ip route add $NAT_SUBNET via $NAT_GWIP

#######################################################################
#Setup intf2 (connect it to the other node via linux bridge)
#######################################################################
brctl addbr br0
brctl stp br0 off
brctl addif br0 $INTF2
brctl addif br0 eth1
ip link set up dev br0
ip link set up dev $INTF2
ip link set up dev eth1

#######################################################################
#Setup host interface (Not used for now)
#######################################################################
#ip link set up dev $HOST0


