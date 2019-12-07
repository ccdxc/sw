#!/bin/sh

counter=300
int_mnic_up=0
oob_mnic_up=0
inb_mnic0_up=0
inb_mnic1_up=0
inb_mnic0_enslaved=0
inb_mnic1_enslaved=0

#setup the bonding device for inband management
ip link s dev bond0 type bond mode active-backup miimon 100

echo "Waiting for mgmt interfaces to show up"

#Wait for mnic interfaces to show up
while true
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

        if [ $inb_mnic0_enslaved -eq 0 ]; then
            echo "adding inb_mnic0 (inband mgmt0) interface to bond0"
            ifconfig bond0 up && ifenslave bond0 inb_mnic0
            [[ $? -eq 0 ]] && inb_mnic0_enslaved=1
        fi

        irq_number=`find /proc/irq  -name *inb_mnic0* | awk -F/ '{ print $4 }'`
        if [[ ! -z $irq_number ]]; then
            echo d > /proc/irq/$irq_number/smp_affinity
            inb_mnic0_up=1
        else 
            sleep 1
        fi
    fi

    if [ -d "/sys/class/net/inb_mnic1" ] && [ $inb_mnic1_up -eq 0 ] ; then
        if [ $inb_mnic1_enslaved -eq 0 ]; then
            echo "adding inb_mnic1 (inband mgmt1) interface to bond0"
            ifconfig bond0 up && ifenslave bond0 inb_mnic1
            [[ $? -eq 0 ]] && inb_mnic1_enslaved=1
        fi

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

