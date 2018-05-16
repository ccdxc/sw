#!/bin/bash

export NIC_DIR=/naples/nic
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LD_LIBRARY_PATH=$NIC_DIR/lib64:$NIC_DIR/lib:$HAL_CONFIG_PATH/sdk:$HAL_CONFIG_PATH/sdk/external:/usr/local/lib:$LD_LIBRARY_PATH
export LOG_DIR=/naples/data/logs
export MODEL_ZMQ_TYPE_TCP=1
export ZMQ_SOC_DIR=$NIC_DIR

echo "Re-Starting hntap ..."
pkill nic_infra_hntap && echo "Stopped hntap" || echo "hntap not running"
echo "Starting hntap ..."
$NIC_DIR/bin/nic_infra_hntap -f $NIC_DIR/conf/hntap-cfg.json > $LOG_DIR/hntap.log  2>&1 &
PID=`ps -eaf | grep nic_infra_hntap | grep -v grep | awk '{print $2}'`
if [[ "" ==  "$PID" ]]; then
    echo "Failed to start hntap service"
    #exit $?
else
    echo "hntap service started, pid is $PID"
fi
