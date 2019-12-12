#!/bin/sh

counter=300
int_mnic_up=0
oob_mnic_up=0
inb_mnic0_up=0
inb_mnic1_up=0

echo "Waiting for mgmt interfaces(int_mnic0 and oob_mnic0) to show up"

#Wait for 5 minutes for int_mnic0 nad oob_mnic0 to show up
while [ $counter -gt 0 ]
do
    if [ -d "/sys/class/net/int_mnic0" ] && [ $int_mnic_up -eq 0 ] ; then
        ifup int_mnic0
        int_mnic_up=1
        irq_number=`find /proc/irq  -name *int_mnic0* | awk -F/ '{ print $4 }'`
        echo d > /proc/irq/$irq_number/smp_affinity
    else
        sleep 1
        counter=$(( $counter - 1 ))
    fi

    if [ -d "/sys/class/net/oob_mnic0" ] && [ $oob_mnic_up -eq 0 ] ; then
        #ifup oob_mnic0
        irq_number=`find /proc/irq  -name *oob_mnic0* | awk -F/ '{ print $4 }'`
        if [[ ! -z $irq_number ]]; then
            echo d > /proc/irq/$irq_number/smp_affinity
            oob_mnic_up=1
        else 
            sleep 1
        fi
    fi

    if [ -d "/sys/class/net/inb_mnic0" ] && [ $inb_mnic0_up -eq 0 ] ; then
        irq_number=`find /proc/irq  -name *inb_mnic0* | awk -F/ '{ print $4 }'`
        if [[ ! -z $irq_number ]]; then
            echo d > /proc/irq/$irq_number/smp_affinity
            inb_mnic0_up=1
        else 
            sleep 1
        fi
    fi

    if [ -d "/sys/class/net/inb_mnic1" ] && [ $inb_mnic1_up -eq 0 ] ; then
        irq_number=`find /proc/irq  -name *inb_mnic1* | awk -F/ '{ print $4 }'`
        if [[ ! -z $irq_number ]]; then
            echo d > /proc/irq/$irq_number/smp_affinity
            inb_mnic1_up=1
        else 
            sleep 1
        fi
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
