#!/bin/bash

int_mnic0_up=0
int_mnic0_admin_up=0

oob_mnic0_up=0
oob_mnic0_admin_up=0

inb_mnic0_up=0
inb_mnic0_admin_up=0
inb_mnic0_enslaved=0

inb_mnic1_up=0
inb_mnic1_admin_up=0
inb_mnic1_enslaved=0

#setup the bonding device for inband management
ip link s dev bond0 type bond mode active-backup miimon 100
mkfifo /tmp/bond0_fifo

log() {
    d=`date '+%Y-%m-%d %H:%M:%S'`
    echo "[$d] $1"
}

debug() {
    log "int_mnic0_up=$int_mnic0_up"
    log "int_mnic0_admin_up=$int_mnic0_admin_up"
    log "oob_mnic0_up=$oob_mnic0_up"
    log "oob_mnic0_admin_up=$oob_mnic0_admin_up"
    log "inb_mnic0_up=$inb_mnic0_up"
    log "inb_mnic0_admin_up=$inb_mnic0_admin_up"
    log "inb_mnic0_enslaved=$inb_mnic0_enslaved"
    log "inb_mnic1_up=$inb_mnic1_up"
    log "inb_mnic1_admin_up=$inb_mnic1_admin_up"
    log "inb_mnic1_enslaved=$inb_mnic1_enslaved"
}

trap debug 0 1 2 3 6

log "Waiting for mgmt interfaces to show up"

#Wait for mnic interfaces to show up
while true
do
    if [ -d "/sys/class/net/int_mnic0" ] && [ $int_mnic0_up -eq 0 ] ; then
        if [ $int_mnic0_admin_up -eq 0 ]; then
	    bus=`/platform/bin/pcieutil dev -D 1dd8:1004`
	    if [ ! -z "$bus" ]; then
		ipaddr="169.254.$bus.1"
		log "bringing up int_mnic0 $ipaddr"
		ifconfig int_mnic0 $ipaddr netmask 255.255.255.0 up && int_mnic0_admin_up=1
	    else
		log "Waiting for ipaddr for int_mnic0"
		ifconfig int_mnic0 up && int_mnic0_admin_up=1
	    fi
        fi

        sysctl -w net.ipv4.conf.int_mnic0.arp_ignore=1

        irq_number=`find /proc/irq  -name *int_mnic0* | awk -F/ '{ print $4 }'`
        if [[ ! -z $irq_number ]]; then
            log "int_mnic0 is up!"
            echo d > /proc/irq/$irq_number/smp_affinity
            int_mnic0_up=1
        fi
    fi

    if [ -d "/sys/class/net/oob_mnic0" ] && [ $oob_mnic0_up -eq 0 ] ; then
        if [ $oob_mnic0_admin_up -eq 0 ]; then
            log "bringing up oob_mnic0"
            ifconfig oob_mnic0 up && oob_mnic0_admin_up=1
            sysctl -w net.ipv4.conf.oob_mnic0.arp_ignore=1
        fi

        irq_number=`find /proc/irq  -name *oob_mnic0* | awk -F/ '{ print $4 }'`
        if [[ ! -z $irq_number ]]; then
            log "oob_mnic0 is up!"
            echo d > /proc/irq/$irq_number/smp_affinity
            oob_mnic0_up=1
        fi
    fi

    # bond0 will keep the mac address of the last slave which got enslaved so we are enslaving inb_mnic0 at last
    if [ -d "/sys/class/net/inb_mnic0" ] && [ $inb_mnic0_up -eq 0 ] && [ $inb_mnic1_up -eq 1 ]; then
        if [ $inb_mnic0_enslaved -eq 0 ]; then
            log "adding inb_mnic0 (inband mgmt0) interface to bond0"
            ifconfig bond0 up && ifenslave bond0 inb_mnic0 && inb_mnic0_enslaved=1
        fi

        if [ $inb_mnic0_admin_up -eq 0 ]; then
            log "bringing up inb_mnic0"
            ifconfig inb_mnic0 up && inb_mnic0_admin_up=1
        fi

        irq_number=`find /proc/irq  -name *inb_mnic0* | awk -F/ '{ print $4 }'`
        if [[ ! -z $irq_number ]]; then
            log "inb_mnic0 is up!"
            echo d > /proc/irq/$irq_number/smp_affinity
            inb_mnic0_up=1
        fi
    fi

    if [ -d "/sys/class/net/inb_mnic1" ] && [ $inb_mnic1_up -eq 0 ]; then
        if [ $inb_mnic1_enslaved -eq 0 ]; then
            log "adding inb_mnic1 (inband mgmt1) interface to bond0"
            ifconfig bond0 up && ifenslave bond0 inb_mnic1 && inb_mnic1_enslaved=1
        fi

        if [ $inb_mnic1_admin_up -eq 0 ]; then
            log "bringing up inb_mnic1"
            ifconfig inb_mnic1 up && inb_mnic1_admin_up=1
        fi

        irq_number=`find /proc/irq  -name *inb_mnic1* | awk -F/ '{ print $4 }'`
        if [[ ! -z $irq_number ]]; then
            log "inb_mnic1 is up!"
            echo d > /proc/irq/$irq_number/smp_affinity
            inb_mnic1_up=1
        fi
    fi

    sleep 1

    if [ $int_mnic0_up -eq 1 ] && [ $oob_mnic0_up -eq 1 ] && [ $inb_mnic0_up -eq 1 ] && [ $inb_mnic1_up -eq 1 ]; then
        if [ -d "/sys/class/net/inb_mnic0/bonding_slave/" ]; then
            inb_mnic0_mac_addr=`cat /sys/class/net/inb_mnic0/bonding_slave/perm_hwaddr`
        else
            inb_mnic0_mac_addr=`cat /sys/class/net/inb_mnic0/address`
        fi

        echo "setting inb_mnic0 mac address as bond0 mac address"
        ifconfig bond0 hw ether $inb_mnic0_mac_addr
        ifconfig bond0

        echo "done" > /tmp/bond0_fifo
        break
    fi

done
