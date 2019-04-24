#!/bin/sh

#Huge-pages for DPDK
echo 256 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
mkdir /dev/hugepages
mount -t hugetlbfs nodev /dev/hugepages

#VPP Partial init env variables
export NIC_DIR=/nic/
export HAL_CONFIG_PATH=$NIC_DIR/conf/

ulimit -c unlimited

exec $NIC_DIR/bin/vpp unix { cli-listen localhost:5002 } heapsize 256M dpdk { vdev net_ionic0 } cpu { main-core 2  corelist-workers 3 }

[[ $? -ne 0 ]] && echo "Failed to start VPP!" && exit 1
