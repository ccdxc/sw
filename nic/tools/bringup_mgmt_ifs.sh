#!/bin/sh

counter=300
int_mnic_up=0

echo "Waiting for mgmt interfaces(int_mnic0 and oob_mnic0) to show up"

#Wait for 5 minutes for int_mnic0 nad oob_mnic0 to show up
while [ $counter -gt 0 ]
do
    if [ -d "/sys/class/net/int_mnic0" ] && [ $int_mnic_up -eq 0 ] ; then
        ifup int_mnic0
        int_mnic_up=1
    else
        sleep 1
        counter=$(( $counter - 1 ))
    fi

    if [ $int_mnic_up -eq 1 ]; then
        break
    fi

done

echo ""

if [ $int_mnic_up -eq 1 ]; then
    echo "Brought up int_mnic0(169.254.0.1) interface for Naples management"
else
    echo "int_mnic0 (mgmt) interface didn't show up for 5 minutes!!!"
fi
