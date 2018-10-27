#!/bin/sh

export NIC_DIR='/nic'
export PLATFORM_DIR='/platform'
export FWD_MODE="$1"
export PLATFORM="$2"

export LD_LIBRARY_PATH=$NIC_DIR/lib:$PLATFORM_DIR/lib

ulimit -c unlimited

cd /
ifconfig lo up

# start memtun
/platform/bin/memtun 0x13b000000 &

# check for all the binaries
if [[ ! -f $NIC_DIR/bin/hal ]]; then
    echo "HAL binary not found"
    exit 1
fi

if [[ ! -f $NIC_DIR/bin/netagent ]]; then
    echo "netagent binary not found"
    exit 1
fi

if [[ ! -f $PLATFORM_DIR/bin/nicmgrd ]]; then
    echo "nicmgr binary not found"
    exit 1
fi

# start sysmgr
rm -f *.log
rm -f agent.log* /tmp/*.db
$NIC_DIR/bin/sysmgr &
[[ $? -ne 0 ]] && echo "Aborting Sysinit - Sysmgr failed to start!" && exit 1

# start HAL
#$NIC_DIR/tools/start-hal-haps.sh
#[[ $? -ne 0 ]] && echo "Aborting Sysinit - HAL failed to start!" && exit 1

# start netagent
# Remove logs
#sleep 20
#$NIC_DIR/bin/netagent -datapath hal -logtofile /agent.log -hostif lo &
#[[ $? -ne 0 ]] && echo "Failed to start AGENT!" && exit 1

#if [[ "$FWD_MODE" != "classic" ]]; then
#    # Remove logs
#    rm -f agent.log* /tmp/*.db
#
#    $NIC_DIR/bin/netagent -datapath hal -logtofile /agent.log -hostif lo &
#    [[ $? -ne 0 ]] && echo "Failed to start AGENT!" && exit 1
#else
#    # create 100G ports in classic mode
#    $NIC_DIR/tools/port_op.sh --create --port 1 --speed 100 --enable 1
#    $NIC_DIR/tools/port_op.sh --create --port 5 --speed 100 --enable 1
#    $NIC_DIR/tools/port_op.sh --create --port 9 --speed 1 --type mgmt --enable 1
#fi

# start nicmgr
#PAL_TRACE='/pal_nicmgr.log' $PLATFORM_DIR/tools/start-nicmgr-haps.sh
#[[ $? -ne 0 ]] && echo "Aborting Sysinit - NICMGR failed to start!" && exit 1

# Renice HAL & LINKMGR so other apps & kernel contexts can run
# renice 20 `pidof hal`
# renice 20 $(ls -1 /proc/`pidof hal`/task)

# Bringup MNIC
# if [[ "$FWD_MODE" == 'classic' ]]; then
#     insmod /platform/ionic_mnic.ko
#     # Pin interrupts so packet processing happens on the control core
#     echo 14 > /proc/irq/9/smp_affinity
#     echo 14 > /proc/irq/10/smp_affinity
#     echo 14 > /sys/class/net/eth0/queues/rx-0/rps_cpus
#     echo 14 > /sys/class/net/eth0/queues/tx-0/xps_cpus
#     ifconfig eth0 10.0.0.1 netmask 255.255.255.0 up
#     ethtool -K eth0 rx off
# fi

echo "All processes brought up, please check ..."
