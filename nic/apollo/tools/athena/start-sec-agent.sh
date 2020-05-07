#!/bin/sh

CUR_DIR=$( readlink -f $( dirname $0 ) )
source $CUR_DIR/setup_env_hw.sh
#export VPP_IPC_MOCK_MODE=1

# remove logs
rm -f $NON_PERSISTENT_LOGDIR/pds-athena-sec-agent.log*

ulimit -c unlimited

export PERSISTENT_LOG_DIR=/obfl_sec/

#Huge-pages for DPDK
echo 64 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
mkdir -p /dev/hugepages
mount -t hugetlbfs nodev /dev/hugepages

exec taskset 1 $PDSPKG_TOPDIR/bin/athena_app -c hal_hw.json -m soft-init $* >$NON_PERSISTENT_LOGDIR/athena_sec_app_console.log &
