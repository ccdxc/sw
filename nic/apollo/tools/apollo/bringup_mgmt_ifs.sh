#!/bin/sh

counter=300
oob_mnic_up=0
inb_mnic0_up=0
inb_mnic1_up=0
int_mnic0_up=0

echo "Waiting for mgmt interfaces to show up"

#Wait for 5 minutes to show up
while [ $counter -gt 0 ]
do
    if [ -d "/sys/class/net/oob_mnic0" ] && [ $oob_mnic_up -eq 0 ] ; then
        # Hack: Till CSUM Offload is supported
        ethtool -K oob_mnic0 rx off tx off
        ifconfig oob_mnic0 up
        dhclient oob_mnic0 > /dev/null 2>&1 &
        oob_mnic_up=1
    fi

    if [ -d "/sys/class/net/inb_mnic0" ] && [ $inb_mnic0_up -eq 0 ] ; then
        ethtool -K inb_mnic0 rx off tx off
        ifconfig inb_mnic0 up
        inb_mnic0_up=1
    fi

    if [ -d "/sys/class/net/inb_mnic1" ] && [ $inb_mnic1_up -eq 0 ] ; then
        ethtool -K inb_mnic1 rx off tx off
        ifconfig inb_mnic1 up
        inb_mnic1_up=1
    fi

    if [ -d "/sys/class/net/int_mnic0" ] && [ $int_mnic0_up -eq 0 ] ; then
        ethtool -K int_mnic0 rx off tx off
        ifconfig int_mnic0 169.254.0.1 netmask 255.255.255.0
        int_mnic0_up=1
    fi

    if [ $oob_mnic_up -eq 1 ] && [ $inb_mnic0_up -eq 1 ] && [ $inb_mnic1_up -eq 1 ] && [ $int_mnic0_up -eq 1 ]; then
        break
    else
        sleep 1
        counter=$(( $counter - 1 ))
    fi

done

echo ""

if [ $oob_mnic_up -eq 1 ] && [ $inb_mnic0_up -eq 1 ] && [ $inb_mnic1_up -eq 1 ] && [ $int_mnic0_up -eq 1 ]; then
    echo "Brought up Management interfaces for Naples management"
else
    echo "Management interfaces didn't show up for 5 minutes!!!"
fi
