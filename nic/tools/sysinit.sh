#!/bin/sh

export NIC_DIR='/nic'
export PLATFORM_DIR='/platform'
export FWD_MODE="$1"
export PLATFORM="$2"
export IMAGE_TYPE="$3"
export GOTRACEBACK='crash'

export LD_LIBRARY_PATH=$NIC_DIR/lib:$PLATFORM_DIR/lib
export COVFILE=$NIC_DIR/conf/hw_bullseye_hal.cov

ulimit -c unlimited

# Reserving port for HAL GRPC server
sysctl -w net.ipv4.ip_local_reserved_ports=50054

# Set core file pattern
CORE_MIN_DISK=512
[[ "$IMAGE_TYPE" = "diag" ]] && CORE_MIN_DISK=1
mkdir -p /data/core
echo "|/nic/bin/coremgr -P /data/core -p %p -e %e -m $CORE_MIN_DISK" > /proc/sys/kernel/core_pattern

# Set the power voltage for the board
/platform/bin/powerctl -set > /obfl/voltage.txt 2>&1

# POST
if [[ -f /sysconfig/config0/post_disable ]]; then
    echo "Skipping Power On Self Test (POST)"
else
    echo "Running Power On Self Test (POST) ..."
    /nic/bin/diag_test post 2>&1 | tee /var/log/pensando/post_report_`date +"%Y%m%d-%T"`.txt
fi

cd /
ifconfig lo up

# start memtun
/platform/bin/memtun &

# if not already present, create a cache file recording the firmware inventory
# at boot-time.  will be preserved across a live-update, and so always
# provides a record of how things looked when we booted (seen with fwupdate -L)
if [[ ! -r /var/run/fwupdate.cache ]]; then
    /nic/tools/fwupdate -C
fi

# check for all the binaries
if [[ ! -f $NIC_DIR/bin/hal ]]; then
    echo "Aborting Sysinit - HAL binary not found"
    exit 1
fi

if [[ ! -f $NIC_DIR/bin/netagent ]]; then
    echo "Aborting Sysinit - netagent binary not found"
    exit 1
fi

if [[ ! -f $PLATFORM_DIR/bin/nicmgrd ]]; then
    echo "Aborting Sysinit - nicmgr binary not found"
    exit 1
fi

if [[ ! -f $PLATFORM_DIR/drivers/ionic_mnic.ko ]]; then
    echo "Aborting Sysinit - mnic driver not found"
    exit 1
fi

if [[ ! -f $PLATFORM_DIR/drivers/mnet.ko ]]; then
    echo "Aborting Sysinit - mnet driver not found"
    exit 1
fi

if [[ ! -f $PLATFORM_DIR/drivers/mnet_uio_pdrv_genirq.ko ]]; then
    echo "Aborting Sysinit - mnet_uio_pdrv_genirq driver not found"
    exit 1
fi

insmod $PLATFORM_DIR/drivers/ionic_mnic.ko &> /var/log/pensando/ionic_mnic_load.log
[[ $? -ne 0 ]] && echo "Aborting Sysinit - Unable to load mnic driver!" && exit 1

insmod $PLATFORM_DIR/drivers/mnet_uio_pdrv_genirq.ko &> /var/log/pensando/mnet_uio_pdrv_genirq_load.log
[[ $? -ne 0 ]] && echo "Aborting Sysinit - Unable to load mnet_uio_pdrv_genirq driver!" && exit 1


insmod $PLATFORM_DIR/drivers/mnet.ko &> /var/log/pensando/mnet_load.log
[[ $? -ne 0 ]] && echo "Aborting Sysinit - Unable to load mnet driver!" && exit 1

# start sysmgr
rm -f *.log
rm -f agent.log* /tmp/*.db
$NIC_DIR/bin/sysmgr /nic/conf/sysmgr.json &
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

$NIC_DIR/tools/bringup_mgmt_ifs.sh &>/var/log/pensando/mgmt_if.log &

echo "All processes brought up, please check ..."
