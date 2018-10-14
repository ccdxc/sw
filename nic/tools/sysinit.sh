#!/bin/sh

export NIC_DIR='/nic'
export PLATFORM_DIR='/platform'
export FWD_MODE="$1"
export PLATFORM="$2"

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

# start HAL
$NIC_DIR/tools/start-hal-haps.sh "$FWD_MODE" "$PLATFORM"
[[ $? -ne 0 ]] && echo "Aborting Sysinit - HAL failed to start!" && exit 1

# start netagent
if [[ "$FWD_MODE" != "classic" ]]; then
    # Remove logs
    rm -f agent.log* /tmp/*.db

    $NIC_DIR/bin/netagent -datapath hal -logtofile /agent.log -hostif lo &
    [[ $? -ne 0 ]] && echo "Failed to start AGENT!" && exit 1

    # workaround for serdes init in smart-nic mode
    export LD_LIBRARY_PATH=$NIC_DIR/lib:$NIC_DIR/conf/sdk:$NIC_DIR/../platform/lib:$NIC_DIR/conf/linkmgr:$NIC_DIR/conf/sdk/external:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
    $NIC_DIR/bin/port_client -o 26 -w 9000
    sleep 1
    /nic/bin/aapl serdes-init -server 0.0.0.0 -port 9000 -addr 34-41 -divider 165 -width-mode 40
    /nic/bin/aapl serdes -server localhost -port 9000 -addr 34-41 -pre 2 -post 6
    /nic/bin/aapl serdes -server 0.0.0.0 -port 9000 -addr 34-41 -interrupt 0xd 0x800f
    /nic/bin/aapl serdes -server 0.0.0.0 -port 9000 -addr 34-41 -interrupt 0xe 0x9e00
    sleep 5
    /nic/bin/aapl serdes -server 0.0.0.0 -port 9000 -addr 34-41 -tx-data core
    /nic/bin/aapl serdes -server 0.0.0.0 -port 9000 -addr 34-41 -rx-input 0

    $NIC_DIR/tools/port_op.sh --update --port 1 --enable 0
    $NIC_DIR/tools/port_op.sh --update --port 5 --enable 0
    $NIC_DIR/tools/port_op.sh --update --port 1 --enable 1
    $NIC_DIR/tools/port_op.sh --update --port 5 --enable 1
else
    # create 100G ports in classic mode
    $NIC_DIR/tools/port_op.sh --create --port 1 --speed 100 --enable 1
    $NIC_DIR/tools/port_op.sh --create --port 5 --speed 100 --enable 1
fi

sleep 5

# start nicmgr
$PLATFORM_DIR/tools/start-nicmgr-haps.sh "$FWD_MODE"
[[ $? -ne 0 ]] && echo "Aborting Sysinit - NICMGR failed to start!" && exit 1

# Renice HAL & LINKMGR so other apps & kernel contexts can run
renice 20 `pidof hal`
renice 20 $(ls -1 /proc/`pidof hal`/task)

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
