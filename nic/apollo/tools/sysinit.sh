#!/bin/sh

export NIC_DIR='/nic'
export LD_LIBRARY_PATH=$NIC_DIR/lib
ulimit -c unlimited
cd /
ifconfig lo up

# load kernel modules for mnics
insmod /nic/bin/ionic_mnic.ko &> /var/log/pensando/ionic_mnic_load.log
[[ $? -ne 0 ]] && echo "Aborting sysinit, failed to load mnic driver!" && exit 1

insmod /nic/bin/mnet_uio_pdrv_genirq.ko &> /var/log/pensando/mnet_uio_pdrv_genirq_load.log
[[ $? -ne 0 ]] && echo "Aborting sysinit, failed to load mnet_uio_pdrv_genirq driver!" && exit 1

insmod /nic/bin/mnet.ko &> /var/log/pensando/mnet_load.log
[[ $? -ne 0 ]] && echo "Aborting sysinit, failed to load mnet driver!" && exit 1

# Set core file pattern
CORE_MIN_DISK=512
[[ "$IMAGE_TYPE" = "diag" ]] && CORE_MIN_DISK=1
mkdir -p /data/core
echo "|/nic/bin/coremgr -P /data/core -p %p -e %e -m $CORE_MIN_DISK" > /proc/sys/kernel/core_pattern

# start agent

if [[ -f $NIC_DIR/tools/start-agent.sh ]]; then
    echo "Launching agent ..."
    $NIC_DIR/tools/start-agent.sh
    [[ $? -ne 0 ]] && echo "Aborting sysinit, failed to start agent!" && exit 1
fi

# start cronjobs
nice crond -c /nic/conf/apollo/crontabs

# if not already present, create a cache file recording the firmware inventory
# at boot-time.  will be preserved across a live-update, and so always
# provides a record of how things looked when we booted (seen with fwupdate -L)
if [[ ! -r /var/run/fwupdate.cache ]]; then
    /nic/tools/fwupdate -C
fi

if [[ -f $NIC_DIR/tools/start-nmd.sh ]]; then
    echo "Launching nmd..."
    $NIC_DIR/tools/start-nmd.sh
    [[ $? -ne 0 ]] && echo "Failed to start nmd!"
fi

echo "System initialization done ..."
