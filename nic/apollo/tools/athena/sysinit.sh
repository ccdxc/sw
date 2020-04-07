#!/bin/sh

CUR_DIR=$( readlink -f $( dirname $0 ) )
source $CUR_DIR/setup_env_hw.sh

export GOTRACEBACK='crash'
export GOGC=50
export SYSMGR_CORES='0x1'
export PERSISTENT_LOG_DIR='/obfl/'

ulimit -c unlimited

# POST
if [[ -f $SYSCONFIG/post_disable ]]; then
    echo "Skipping Power On Self Test (POST)"
else
    echo "Running Power On Self Test (POST) ..."
    ($PDSPKG_TOPDIR/bin/diag_test post 2>&1 > $NON_PERSISTENT_LOGDIR/post_report_`date +"%Y%m%d-%T"`.txt; echo > /tmp/.post_done) &
fi

cd /
ifconfig lo up

sysctl -w net.ipv4.ip_local_reserved_ports=50054,9007

# load kernel modules for mnics
insmod $PDSPKG_TOPDIR/bin/ionic_mnic.ko &> $NON_PERSISTENT_LOGDIR/ionic_mnic_load.log
[[ $? -ne 0 ]] && echo "Aborting sysinit, failed to load mnic driver!" && exit 1

insmod $PDSPKG_TOPDIR/bin/mnet_uio_pdrv_genirq.ko &> $NON_PERSISTENT_LOGDIR/mnet_uio_pdrv_genirq_load.log
[[ $? -ne 0 ]] && echo "Aborting sysinit, failed to load mnet_uio_pdrv_genirq driver!" && exit 1

insmod $PDSPKG_TOPDIR/bin/mnet.ko &> $NON_PERSISTENT_LOGDIR/mnet_load.log
[[ $? -ne 0 ]] && echo "Aborting sysinit, failed to load mnet driver!" && exit 1

# set core file pattern
CORE_MIN_DISK=512
[[ "$IMAGE_TYPE" = "diag" ]] && CORE_MIN_DISK=1
mkdir -p /data/core
echo "|$PDSPKG_TOPDIR/bin/coremgr -P /data/core -p %p -e %e -m $CORE_MIN_DISK" > /proc/sys/kernel/core_pattern

# start memtun
[ -f $SYSCONFIG/memtun_enable ] && (taskset 1 /$PDSPKG_TOPDIR/bin/memtun &)

# if not already present, create a cache file recording the firmware inventory
# at boot-time.  will be preserved across a live-update, and so always
# provides a record of how things looked when we booted (seen with fwupdate -L)
if [[ ! -r /var/run/fwupdate.cache ]]; then
    $PDSPKG_TOPDIR/tools/fwupdate -C &
fi

# Sync the boot fault log
if [[ -f $PDSPKG_TOPDIR/tools/bflog_sync.sh ]]; then
    $PDSPKG_TOPDIR/tools/bflog_sync.sh &
fi

# start sysmgr
PENLOG_LOCATION=$PERSISTENT_LOGDIR $PDSPKG_TOPDIR/bin/sysmgr &

# bring up oob
echo "Bringing up internal mnic interfaces ..."
$PDSPKG_TOPDIR/tools/bringup_mgmt_ifs.sh &> $NON_PERSISTENT_LOGDIR/mgmt_if.log

# start cronjobs
nice crond -c $PDSPKG_TOPDIR/conf/athena/crontabs

echo "System initialization done ..."
