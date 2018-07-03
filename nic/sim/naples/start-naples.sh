#!/bin/bash

echo "Bringing up NAPLES services/processes ..."

export VER=v1
export NIC_DIR=/naples/nic
export LOG_DIR=/naples/data/logs
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export HAL_LOG_DIR=$LOG_DIR
export LD_LIBRARY_PATH=$NIC_DIR/lib64:$NIC_DIR/lib:$HAL_CONFIG_PATH/sdk:$HAL_CONFIG_PATH/sdk/external:/usr/local/lib:$LD_LIBRARY_PATH
export MODEL_ZMQ_TYPE_TCP=1
export ZMQ_SOC_DIR=$NIC_DIR
ulimit -c unlimited

# create directory for logs/traces
mkdir -p $LOG_DIR
exec > $LOG_DIR/start-naples.log
exec 2>&1
set -x

if [ -f /tmp/naples-netagent.db ]; then
    rm -f /tmp/naples-netagent.db
fi

# make the example configs visible to the VM outside
if [ -d /naples/nic/data/examples ]; then
    mkdir -p /naples/data/examples/
    mkdir -p /naples/data/examples/config_ipsec/
    mkdir -p /naples/data/examples/config_vxlan_overlay/
    cp /naples/nic/tools/bootstrap.sh /naples/data/
    cp -r /naples/nic/data/examples/* /naples/data/examples/
    cp -r /naples/nic/data/examples/config_ipsec/* /naples/data/examples/config_ipsec/
    cp -r /naples/nic/data/examples/config_vxlan_overlay/* /naples/data/examples/config_vxlan_overlay/
fi

# starting the processes from log directory so that cores are saved there
cd "$LOG_DIR"

echo "Starting CAPRI model ..."
#$NIC_DIR/bin/cap_model +PLOG_MAX_QUIT_COUNT=0 +plog=info +model_debug=$HAL_CONFIG_PATH/iris/model_debug.json > $LOG_DIR/model.log 2>&1 &
$NIC_DIR/bin/cap_model +PLOG_MAX_QUIT_COUNT=0 > /dev/null 2>&1 &
PID=`ps -eaf | grep cap_model | grep -v grep | awk '{print $2}'`
if [[ "" ==  "$PID" ]]; then
    echo "Failed to start CAPRI model"
    #exit $?
else
    echo "CAPRI model started, pid is $PID"
fi

echo "Starting HAL ..."
"$NIC_DIR"/bin/hal -c hal.json > /dev/null 2>&1 &

# wait for HAL to open gRPC port before spawning agent(s)
HAL_WAIT_TIMEOUT=1
HAL_GRPC_PORT="${HAL_GRPC_PORT:-50054}"
HAL_SERVER="localhost:$HAL_GRPC_PORT"
HAL_UP=-1
MAX_RETRIES=600
i=0
until (( HAL_UP == 0 )) || (( i == MAX_RETRIES ))
do
    echo "Waiting for HAL GRPC server to be up ..."
    sleep "$HAL_WAIT_TIMEOUT"
    curl "$HAL_SERVER"
    HAL_UP="$?"
    if [ "$HAL_UP" -eq 0 ]; then
        echo "HAL is up"
    else
        echo "HAL not up yet"
    fi
    let "i++"
done

if [ "$i" -eq "$MAX_RETRIES" ]; then
    echo "HAL server failed to come up"
    exit 1
fi

# start NIC manager and allow it to create uplinks
echo "Starting nicmgr ..."
"$NIC_DIR"/bin/nic_mgr_app
if [ $? -ne 0 ]; then
    echo "Failed to start nic mgr"
    exit $?
fi

echo "Starting hntap ..."
"$NIC_DIR"/bin/nic_infra_hntap -f $NIC_DIR/conf/hntap-cfg.json &

echo "Starting netagent ..."
"$NIC_DIR"/bin/netagent -hostif lo -logtofile $LOG_DIR/agent.log -datapath hal &

echo "NAPLES services/processes up and running ..."

# keep the container running
while :; do
      sleep 300
done
