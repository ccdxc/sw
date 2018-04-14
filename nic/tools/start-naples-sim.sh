#!/bin/bash

echo "Bringing up NAPLES processes ..."
NIC_DIR=/naples/nic
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LD_LIBRARY_PATH=$NIC_DIR/lib64:$NIC_DIR/lib:/usr/local/lib:$LD_LIBRARY_PATH
export MODEL_ZMQ_TYPE_TCP=1
ulimit -c unlimited

echo "Starting Model ..."
export ZMQ_SOC_DIR=$NIC_DIR
$NIC_DIR/bin/cap_model +PLOG_MAX_QUIT_COUNT=0 +plog=info +model_debug=$NIC_DIR/gen/iris/dbg_out/model_debug.json 2>&1 | tee $NIC_DIR/model.log &

echo "Starting HAL ..."
$NIC_DIR/bin/hal -c hal.json &

echo "Starting Agent ..."
LD_LIBRARY_PATH=lib64:$LD_LIBRARY_PATH $NIC_DIR/bin/netagent -hostif lo -logtofile agent.log -datapath mock &

echo "NAPLES up and running ..."

# keep the container running
while :; do
      sleep 300
done
