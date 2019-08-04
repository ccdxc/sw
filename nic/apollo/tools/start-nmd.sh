#!/bin/sh

counter=300
int_mnic_up=0
oob_mnic_up=0
inb_mnic0_up=0
inb_mnic1_up=0

#setup the bonding device for inband management
ip link s dev bond0 type bond mode active-backup miimon 100
ip link s dev bond0 up

echo "Waiting for mgmt interfaces(int_mnic0 and oob_mnic0) to show up"

#Wait for 5 minutes for int_mnic0 and oob_mnic0 to show up
while [ $counter -gt 0 ]
do
    if [ -d "/sys/class/net/int_mnic0" ] && [ $int_mnic_up -eq 0 ] ; then
        ifup int_mnic0
        int_mnic_up=1
    elif [ -d "/sys/class/net/oob_mnic0" ] && [ $oob_mnic_up -eq 0 ] ; then
        echo "OOB up"
        oob_mnic_up=1
    elif [ -d "/sys/class/net/inb_mnic0" ] && [ $inb_mnic0_up -eq 0 ] ; then
        inb_mnic0_up=1
    elif [ -d "/sys/class/net/inb_mnic1" ] && [ $inb_mnic1_up -eq 0 ] ; then
        inb_mnic1_up=1
    else
        sleep 1
        counter=$(( $counter - 1 ))
    fi

    if [ $int_mnic_up -eq 1 ] && [ $oob_mnic_up -eq 1 ] && [ $inb_mnic0_up -eq 1 ] && [ $inb_mnic1_up -eq 1 ]; then
        break
    fi

done

echo ""

if [ $int_mnic_up -eq 1 ]; then
    echo "Brought up int_mnic0(169.254.0.1) interface for Naples management"
else
    echo "int_mnic0 (mgmt) interface didn't show up for 5 minutes!!!"
fi

if [ $oob_mnic_up -eq 1 ]; then
    echo "Brought up oob_mnic0(DHCP) interface for Naples management"
else
    echo "oob_mnic0 (mgmt) interface didn't show up for 5 minutes!!!"
fi

if [ $inb_mnic0_up -eq 1 ]; then
    echo "adding inb_mnic0 (inband mgmt0) interface to bond0"
    ifenslave bond0 inb_mnic0
else
    echo "inb_mnic0 interface didn't show up!!!"
fi

if [ $inb_mnic1_up -eq 1 ]; then
    echo "inb_mnic1 (inband mgmt1) interface added to bond0"
    ifenslave bond0 inb_mnic1
else
    echo "inb_mnic1 interface didn't show up!!!"
fi

# Temporary dhclient
ifconfig bond0 down
kill -9 $(pidof dhclient)
exec env TMPDIR=/data NAPLES_PIPELINE=apollo /nic/bin/nmd
