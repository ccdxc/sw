#!/bin/sh

export NIC_DIR='/nic'
export LD_LIBRARY_PATH=$NIC_DIR/lib
export CONFIG_PATH=$NIC_DIR/conf/
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LIBRARY_PATH=$NIC_DIR/lib:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
export HAL_PBC_INIT_CONFIG="2x100_hbm"
export COVFILE=$NIC_DIR/coverage/sim_bullseye_hal.cov

export SYSCONFIG='/sysconfig/config0'
export LOG_DIR='/var/log/pensando/'
export PERSISTENT_LOGDIR='/obfl/'
export NON_PERSISTENT_LOGDIR='/var/log/pensando/'
export GOTRACEBACK='crash'
export GOGC=50

ulimit -c unlimited

# POST
if [[ -f $SYSCONFIG/post_disable ]]; then
    echo "Skipping Power On Self Test (POST)"
else
    echo "Running Power On Self Test (POST) ..."
    ($NIC_DIR/bin/diag_test post 2>&1 > $NON_PERSISTENT_LOGDIR/post_report_`date +"%Y%m%d-%T"`.txt; echo > /tmp/.post_done) &
fi

cd /
ifconfig lo up

sysctl -w net.ipv4.ip_local_reserved_ports=50054,9007

# load kernel modules for mnics
insmod $NIC_DIR/bin/ionic_mnic.ko &> $NON_PERSISTENT_LOGDIR/ionic_mnic_load.log
[[ $? -ne 0 ]] && echo "Aborting sysinit, failed to load mnic driver!" && exit 1

insmod $NIC_DIR/bin/mnet_uio_pdrv_genirq.ko &> $NON_PERSISTENT_LOGDIR/mnet_uio_pdrv_genirq_load.log
[[ $? -ne 0 ]] && echo "Aborting sysinit, failed to load mnet_uio_pdrv_genirq driver!" && exit 1

insmod $NIC_DIR/bin/mnet.ko &> $NON_PERSISTENT_LOGDIR/mnet_load.log
[[ $? -ne 0 ]] && echo "Aborting sysinit, failed to load mnet driver!" && exit 1

# set core file pattern
CORE_MIN_DISK=512
[[ "$IMAGE_TYPE" = "diag" ]] && CORE_MIN_DISK=1
mkdir -p /data/core
echo "|$NIC_DIR/bin/coremgr -P /data/core -p %p -e %e -m $CORE_MIN_DISK" > /proc/sys/kernel/core_pattern

# start memtun
[ -f $SYCONFIG/memtun_enable ] && (/$NIC_DIR/bin/memtun &)

# if not already present, create a cache file recording the firmware inventory
# at boot-time.  will be preserved across a live-update, and so always
# provides a record of how things looked when we booted (seen with fwupdate -L)
if [[ ! -r /var/run/fwupdate.cache ]]; then
    $NIC_DIR/tools/fwupdate -C &
fi

# Sync the boot fault log
if [[ -f $NIC_DIR/tools/bflog_sync.sh ]]; then
    $NIC_DIR/tools/bflog_sync.sh &
fi

# start sysmgr
PENLOG_LOCATION=/obfl $NIC_DIR/bin/sysmgr &

# bring up oob
echo "Bringing up internal mnic interfaces ..."
$NIC_DIR/tools/bringup_mgmt_ifs.sh &> $NON_PERSISTENT_LOGDIR/mgmt_if.log

# start cronjobs
nice crond -c $NIC_DIR/conf/apollo/crontabs

echo "System initialization done ..."
