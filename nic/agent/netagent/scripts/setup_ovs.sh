#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Invalid number of arguments"
    echo " $0 <ip-addr-suffix>"
    exit -1
fi

# cleanup previous state
sudo ip link delete strunk0 type veth peer name ntrunk0
sudo ovs-vsctl del-br br0

# create new state
sudo ovs-vsctl add-br br0
sudo ovs-vsctl add-port br0 vlan2 -- set interface vlan2 type=internal
sudo ovs-vsctl set port vlan2 tag=2
sudo ifconfig vlan2 hw ether 01:02:02:$1:$1:$1 up
sudo ifconfig vlan2 10.1.2.$1/24 up
sudo ovs-vsctl add-port br0 vlan3 -- set interface vlan3 type=internal
sudo ovs-vsctl set port vlan3 tag=3
sudo ifconfig vlan2 hw ether 01:03:03:$1:$1:$1 up
sudo ifconfig vlan3 10.1.3.$1/24 up
sudo ip link add strunk0 type veth peer name ntrunk0
sudo ifconfig strunk0 up
sudo ifconfig ntrunk0 up
sudo ifconfig ntrunk0 hw ether 01:01:01:$1:$1:$1 up
sudo ovs-vsctl add-port br0 strunk0
sudo ovs-vsctl set port strunk0 vlan_mode=trunk
sudo ifconfig ntrunk0 promisc up
