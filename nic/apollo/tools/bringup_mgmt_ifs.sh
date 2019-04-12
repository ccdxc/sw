#!/bin/sh

counter=300
oob_mnic_up=0

echo "Waiting for mgmt interface  oob_mnic0 to show up"

#Wait for 5 minutes for oob_mnic0 to show up
while [ $counter -gt 0 ]
do
    if [ -d "/sys/class/net/oob_mnic0" ] && [ $oob_mnic_up -eq 0 ] ; then
        # Hack: Till CSUM Offload is supported
        ethtool -K oob_mnic0 rx off tx off
        ifconfig oob_mnic0 up
        dhclient oob_mnic0 &
        oob_mnic_up=1
    else
        sleep 1
        counter=$(( $counter - 1 ))
    fi

    if [ $oob_mnic_up -eq 1 ]; then
        break
    fi

done

echo ""

if [ $oob_mnic_up -eq 1 ]; then
    echo "Brought up Out of Band(OOB) Management interface for Naples management"
else
    echo "Out of Band(OOB) Management interface didn't show up for 5 minutes!!!"
fi
