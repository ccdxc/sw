#!/bin/sh

counter=300
oob0_up=0
inb_mnic0_up=0
inb_mnic1_up=0

while [ $counter -gt 0 ]
do
    if [ -d "/sys/class/net/oob_mnic0" ] && [ $oob0_up -eq 0 ] ; then
        ethtool -K oob_mnic0 rx off tx off
        ifconfig oob_mnic0 up
        irq_number=`find /proc/irq  -name *oob_mnic0* | awk -F/ '{ print $4 }'`
        if [[ ! -z $irq_number ]]; then
            echo d > /proc/irq/$irq_number/smp_affinity
            oob0_up=1
        fi
        echo "oob interface is up"
        dhclient oob_mnic0 > /dev/null 2>&1 &
    fi

    if [ -d "/sys/class/net/inb_mnic0" ] && [ $inb_mnic0_up -eq 0 ] ; then
        ethtool -K inb_mnic0 rx off tx off
        ifconfig inb_mnic0 up
        irq_number=`find /proc/irq  -name *inb_mnic0* | awk -F/ '{ print $4 }'`
        if [[ ! -z $irq_number ]]; then
            echo 3 > /proc/irq/$irq_number/smp_affinity
            inb_mnic0_up=1
        fi
        echo "inb_mnic0 interface is up"
    fi

    if [ -d "/sys/class/net/inb_mnic1" ] && [ $inb_mnic1_up -eq 0 ] ; then
        ethtool -K inb_mnic1 rx off tx off
        ifconfig inb_mnic1 up && inb_mnic1_up=1
        irq_number=`find /proc/irq  -name *inb_mnic1* | awk -F/ '{ print $4 }'`
        if [[ ! -z $irq_number ]]; then
            echo 3 > /proc/irq/$irq_number/smp_affinity
            inb_mnic1_up=1
        fi
        echo "inb_mnic1 interface is up"
    fi

    if [ $oob0_up -eq 1 ] && [ $inb_mnic0_up -eq 1 ] && [ $inb_mnic1_up -eq 1 ]; then
        break
    else
        echo "Waiting for mgmt interfaces to be created ..."
        sleep 1
        counter=$(( $counter - 1 ))
    fi
done

echo ""
if [ $oob0_up -eq 1 ] && [ $inb_mnic0_up -eq 1 ] && [ $inb_mnic1_up -eq 1 ]; then
    echo "All internal interfaces are brought up"
else
    echo "All internal interfaces didn't show up for 5 minutes !!"
fi
