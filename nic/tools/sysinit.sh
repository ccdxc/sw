#!/bin/sh

export NIC_DIR='/nic'
export PLATFORM_DIR='/platform'
export FWD_MODE="$1"
export PLATFORM="$2"

ulimit -c unlimited

cd /
ifconfig lo up

# Mount debugfs
mount -t debugfs debugfs /sys/kernel/debug/

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

if [[ "$PLATFORM" == 'hw' ]]; then
    # start linkmgr
    $NIC_DIR/tools/start-linkmgr-hw.sh
    #[[ $? -ne 0 ]] && echo "Aborting Sysinit - LINKMGR failed to start!" && exit 1
fi

# start HAL
$NIC_DIR/tools/start-hal-haps.sh "$FWD_MODE" "$PLATFORM"
[[ $? -ne 0 ]] && echo "Aborting Sysinit - HAL failed to start!" && exit 1

# start nicmgr
$PLATFORM_DIR/tools/start-nicmgr-haps.sh "$FWD_MODE"
[[ $? -ne 0 ]] && echo "Aborting Sysinit - NICMGR failed to start!" && exit 1

# start memtun
/platform/bin/memtun 0x13b000000 &

# Renice HAL & LINKMGR so other apps & kernel contexts can run
renice 20 `pidof hal`
renice 20 $(ls -1 /proc/`pidof hal`/task)
renice 20 `pidof linkmgr`
renice 20 $(ls -1 /proc/`pidof linkmgr`/task)

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

# start netagent
if [[ "$FWD_MODE" != "classic" ]]; then
    # Remove logs
    rm -f agent.log* /tmp/*.db

    $NIC_DIR/bin/netagent -datapath hal -logtofile /agent.log -hostif lo &
    [[ $? -ne 0 ]] && echo "Failed to start AGENT!" && exit 1
fi

$NIC_DIR/tools/port_init_100g.sh

echo "All processes brought up, please check ..."
