#!/bin/sh

# wait for 5 minutes for interfaces to show up
counter=300
oob_up=0
dsc0_up=0
dsc1_up=0
while [ $counter -gt 0 ]
do
    echo $dsc0_up
    echo $dsc1_up
    if [ -d "/sys/class/net/oob_mnic0" ] && [ $oob_up -eq 0 ] ; then
        echo "OOB up"
        oob_up=1
    fi
    if [ -d "/sys/class/net/dsc0" ] && [ $dsc0_up -eq 0 ] ; then
        echo "dsc0 up"
        dsc0_up=1
    fi
    if [ -d "/sys/class/net/dsc1" ] && [ $dsc1_up -eq 0 ] ; then
        echo "dsc1 up"
        dsc1_up=1
    fi

    if [ $oob_up -eq 1 ] && [ $dsc0_up -eq 1 ] && [ $dsc1_up -eq 1 ]; then
        break
    else
        echo "one of the interfaces didn't come up"
        echo "oob status $oob_up"
        echo "dsc0 status $dsc0"
        echo "dsc1 status $dsc1"
        sleep 1
        counter=$(( $counter - 1 ))
    fi
done

exec env TMPDIR=/data NAPLES_PIPELINE=apollo /nic/bin/nmd
