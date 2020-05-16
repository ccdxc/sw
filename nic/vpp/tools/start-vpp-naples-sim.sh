#!/bin/sh

#VPP Partial init env variables
export PDSPKG_TOPDIR=$NIC_DIR
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export CONFIG_PATH=$NIC_DIR/conf/
export VPP_LOG_FILE=/var/log/pensando/vpp.log
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/naples/usr/lib/
export ASIC_MOCK_MODE=1
export ASIC_MOCK_MEMORY_MODE=1
export ZMQ_SOC_DIR=$NIC_DIR/

ulimit -c unlimited

exec $NIC_DIR/bin/vpp -c $HAL_CONFIG_PATH/vpp/vpp_1_worker_naples_sim.conf
