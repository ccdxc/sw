#!/bin/sh
counter=300
int_mnic_up=0
oob_mnic_up=0
inb_mnic0_up=0
inb_mnic1_up=0

echo "Waiting for mgmt interfaces(int_mnic0 and oob_mnic0) to show up"

#Wait for 5 minutes for int_mnic0 and oob_mnic0 to show up
while [ $counter -gt 0 ]
do
    if [ -d "/sys/class/net/int_mnic0" ] && [ $int_mnic_up -eq 0 ] ; then
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

ifconfig bond0 down
kill -9 $(pidof dhclient)
exec env TMPDIR=/data /nic/bin/nmd
