#!/bin/sh

export NIC_DIR='/nic'
export LD_LIBRARY_PATH=$NIC_DIR/lib
SYSCONFIG=/sysconfig/config0
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
export NIC_DIR=`dirname $ABS_TOOLS_DIR`
export CONFIG_PATH=$NIC_DIR/conf/
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LOG_DIR=/var/log/pensando/
export LIBRARY_PATH=$NIC_DIR/lib:$NIC_DIR/../platform/lib:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
export HAL_PBC_INIT_CONFIG="2x100_hbm"
export COVFILE=$NIC_DIR/coverage/sim_bullseye_hal.cov
export AGENT_TEST_HOOKS_LIB=libflowtestagenthooks.so
export PERSISTENT_LOG_DIR='/obfl/'

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

# start memtun
/nic/bin/memtun &

# start sysmgr
PENLOG_LOCATION=/obfl $NIC_DIR/bin/sysmgr &

# bring up oob
sleep 2
echo "Bringing up OOB/Inband/Internal-Management IFs ..."
$NIC_DIR/tools/bringup_mgmt_ifs.sh &> /var/log/pensando/mgmt_if.log

# start cronjobs
nice crond -c /nic/conf/apollo/crontabs

echo "System initialization done ..."
