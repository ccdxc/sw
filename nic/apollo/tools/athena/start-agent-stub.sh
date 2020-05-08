#!/bin/sh

# start memtun
[ ! -f $SYSCONFIG/memtun_enable ] && (taskset 1 $PDSPKG_TOPDIR/bin/memtun &)

echo "Execing Athena App"
exec taskset 1 /nic/tools/start-agent-skip-dpdk.sh
