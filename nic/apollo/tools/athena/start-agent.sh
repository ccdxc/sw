#!/bin/sh

CUR_DIR=$( readlink -f $( dirname $0 ) )
source $CUR_DIR/setup_env_hw.sh
#export IPC_MOCK_MODE=1

# remove logs
rm -f $NON_PERSISTENT_LOG_DIR/pds-athena-agent.log*

ulimit -c unlimited

export PERSISTENT_LOG_DIR=/obfl/

#Huge-pages for DPDK
if  ! mount | grep hugetlbfs ; then
    echo 64 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
    mkdir -p /dev/hugepages
    mount -t hugetlbfs nodev /dev/hugepages
fi

exec taskset 1 $PDSPKG_TOPDIR/bin/athena_app -c hal_hw.json -m cpp $* >$NON_PERSISTENT_LOG_DIR/athena_app_console.log &
