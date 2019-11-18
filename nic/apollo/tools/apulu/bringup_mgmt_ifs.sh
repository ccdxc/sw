#!/bin/sh

counter=300
oob0=0
eth0_up=0
eth1_up=0

while [ $counter -gt 0 ]
do
    if [ -d "/sys/class/net/oob0" ] && [ $oob0 -eq 0 ] ; then
        # Hack: till checksum offload is supported
        ethtool -K oob0 rx off tx off
        ifconfig oob0 up
        dhclient oob0 > /dev/null 2>&1 &
        oob0=1
    fi

    if [ -d "/sys/class/net/eth0" ] && [ $eth0_up -eq 0 ] ; then
        ethtool -K eth0 rx off tx off
        ifconfig eth0 up
        eth0_up=1
    fi

    if [ -d "/sys/class/net/eth1" ] && [ $eth1_up -eq 0 ] ; then
        ethtool -K eth1 rx off tx off
        ifconfig eth1 up
        eth1_up=1
    fi

    if [ $oob0 -eq 1 ] && [ $eth0_up -eq 1 ] && [ $eth1_up -eq 1 ]; then
        break
    else
        echo "Waiting for mnic interfaces to be created ..."
        sleep 1
        counter=$(( $counter - 1 ))
    fi
done

echo ""
if [ $oob0 -eq 1 ] && [ $eth0_up -eq 1 ] && [ $eth1_up -eq 1 ]; then
    echo "All internal interfaces are brought up"
else
    echo "All internal interfaces didn't show up for 5 minutes !!"
fi
