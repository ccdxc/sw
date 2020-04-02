#!/bin/sh

CUR_DIR=$( readlink -f $( dirname $0 ) )
source $CUR_DIR/setup_env_hw.sh
#export VPP_IPC_MOCK_MODE=1

# remove logs
rm -f $NON_PERSISTENT_LOGDIR/pds-athena-agent.log*

ulimit -c unlimited

#Huge-pages for DPDK
#echo 64 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
#mkdir -p /dev/hugepages
#mount -t hugetlbfs nodev /dev/hugepages

exec taskset 1 $PDSPKG_TOPDIR/bin/athena_app -c hal_hw.json -m no-dpdk $* >$NON_PERSISTENT_LOGDIR/athena_app_console.log &
