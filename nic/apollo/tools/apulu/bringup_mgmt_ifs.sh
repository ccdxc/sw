#!/bin/sh

counter=300
oob_mnic0=0
dsc0_up=0
dsc1_up=0

while [ $counter -gt 0 ]
do
    if [ -d "/sys/class/net/oob_mnic0" ] && [ $oob_mnic0 -eq 0 ] ; then
        # Hack: till checksum offload is supported
        ethtool -K oob_mnic0 rx off tx off
        ifconfig oob_mnic0 up
        dhclient oob_mnic0 > /dev/null 2>&1 &
        oob_mnic0=1
    fi

    if [ -d "/sys/class/net/dsc0" ] && [ $dsc0_up -eq 0 ] ; then
        ethtool -K dsc0 rx off tx off
        ifconfig dsc0 up
        dsc0_up=1
    fi

    if [ -d "/sys/class/net/dsc1" ] && [ $dsc1_up -eq 0 ] ; then
        ethtool -K dsc1 rx off tx off
        ifconfig dsc1 up
        dsc1_up=1
    fi

    if [ $oob_mnic0 -eq 1 ] && [ $dsc0_up -eq 1 ] && [ $dsc1_up -eq 1 ]; then
        break
    else
        echo "Waiting for mnic interfaces to be created ..."
        sleep 1
        counter=$(( $counter - 1 ))
    fi
done

echo ""
if [ $oob_mnic0 -eq 1 ] && [ $dsc0_up -eq 1 ] && [ $dsc1_up -eq 1 ]; then
    echo "All internal interfaces are brought up"
else
    echo "All internal interfaces didn't show up for 5 minutes !!"
fi
