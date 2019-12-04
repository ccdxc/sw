#!/bin/sh

NICDIR=$1
PIPELINE=$2

#set -x
#echo $NICDIR

#Huge-pages for DPDK
echo 4096 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
mkdir /dev/hugepages
mount -t hugetlbfs nodev /dev/hugepages

#VPP Partial init env variables
export HAL_CONFIG_PATH=$NICDIR/conf/
export ZMQ_SOC_DIR=$NICDIR
#This is used in DPDK to chosing DESC/Pkt buffer memory pool
export DPDK_SIM_APP_ID=1

ulimit -c unlimited

VPP_PKG_DIR=$NICDIR/sdk/third-party/vpp-pkg/x86_64

#echo "$VPP_PKG_DIR"
#setup libs required for vpp
rm -rf /usr/lib/vpp_plugins/*
mkdir -p /usr/lib/vpp_plugins/
ln -s $VPP_PKG_DIR/lib/vpp_plugins/dpdk_plugin.so /usr/lib/vpp_plugins/dpdk_plugin.so
#Create softlink for all vpp plugins
find $NICDIR/vpp/ -name *.mk | xargs grep MODULE_TARGET | grep "\.so" | awk '{ print $3 }' | xargs -I@ bash -c "ln -s $NICDIR/build/x86_64/$PIPELINE/lib/@ /usr/lib/vpp_plugins/@"

