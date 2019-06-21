#!/bin/sh

#Huge-pages for DPDK
echo 256 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
mkdir /dev/hugepages
mount -t hugetlbfs nodev /dev/hugepages

#VPP Partial init env variables
export NIC_DIR=/nic/
export HAL_CONFIG_PATH=$NIC_DIR/conf/

ulimit -c unlimited

exec $NIC_DIR/bin/vpp -c $HAL_CONFIG_PATH/vpp/vpp_1_worker.conf &
[[ $? -ne 0 ]] && echo "Failed to start VPP!" && exit 1
