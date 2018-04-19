#!/bin/bash

echo "Bringing up NAPLES services/processes ..."

export NIC_DIR=/naples/nic
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LD_LIBRARY_PATH=$NIC_DIR/lib64:$NIC_DIR/lib:/usr/local/lib:$LD_LIBRARY_PATH
export MODEL_ZMQ_TYPE_TCP=1
export ZMQ_SOC_DIR=$NIC_DIR
ulimit -c unlimited

echo "Starting NAPLES model ..."
$NIC_DIR/bin/cap_model +PLOG_MAX_QUIT_COUNT=0 +plog=info +model_debug=$NIC_DIR/gen/iris/dbg_out/model_debug.json 2>&1 | tee $NIC_DIR/model.log &
PID=`ps -eaf | grep cap_model | grep -v grep | awk '{print $2}'`
if [[ "" ==  "$PID" ]]; then
    echo "Failed to start NAPLES model"
    #exit $?
else
    echo "NAPLES model started, pid is $PID"
fi

echo "Starting HAL ..."
$NIC_DIR/bin/hal -c hal.json &
PID=`ps -eaf | grep hal | grep -v grep | awk '{print $2}'`
if [[ "" ==  "$PID" ]]; then
    echo "Failed to start HAL"
    #exit $?
else
    echo "HAL started, pid is $PID"
fi

echo "Starting hntap ..."
$NIC_DIR/bin/nic_infra_hntap -f $NIC_DIR/conf/hntap-cfg.json &
PID=`ps -eaf | grep nic_infra_hntap | grep -v grep | awk '{print $2}'`
if [[ "" ==  "$PID" ]]; then
    echo "Failed to start hntap service"
    #exit $?
else
    echo "hntap service started, pid is $PID"
fi

echo "Starting netagent ..."
$NIC_DIR/bin/netagent -hostif lo -logtofile agent.log -datapath mock &
PID=`ps -eaf | grep nic_infra_hntap | grep -v grep | awk '{print $2}'`
if [[ "" ==  "$PID" ]]; then
    echo "Failed to start netagent"
    #exit $?
else
    echo "netagent service started, pid is $PID"
fi

echo "NAPLES services/processes up and running ..."

# keep the container running
while :; do
      sleep 300
done
