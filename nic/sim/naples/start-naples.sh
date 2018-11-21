#!/bin/bash

echo "Bringing up NAPLES services/processes ..."
set -x

export VER=v1
export NIC_DIR=/naples/nic
export PLATFORM_DIR=/naples/platform
export LOG_DIR=/naples/data/logs
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export HAL_LOG_DIR=$LOG_DIR
export LD_LIBRARY_PATH=$NIC_DIR/lib64:$NIC_DIR/lib:$HAL_CONFIG_PATH/plugins/lif:$HAL_CONFIG_PATH/linkmgr:$HAL_CONFIG_PATH/sdk:$HAL_CONFIG_PATH/sdk/external:usr/local/lib::$LD_LIBRARY_PATH
export NICMGR_LIBRARY_PATH=$PLATFORM_DIR/lib:$NIC_DIR/lib:$NIC_DIR/conf/sdk:$LD_LIBRARY_PATH
export NICMGR_CONFIG_PATH=$PLATFORM_DIR/etc/nicmgrd
unset MODEL_ZMQ_TYPE_TCP
export ZMQ_SOC_DIR=$NIC_DIR
export DISABLE_AGING=1
export CMD_RESOLVER_PORT=9009
export CMD_GRPC_UNAUTH_PORT=9002
export NPM_RPC_PORT=9005

if [ -z "$WITH_QEMU" ]; then
    WITH_QEMU=0
    echo "Running naples-sim in standalone mode"
else
    WITH_QEMU=1
    echo "Running naples-sim with Qemu"
fi

if [ -z "$NMD_HOSTNAME" ]; then
    NMD_HOSTNAME=""
    echo "Nmd Hostname not set"
else
    echo "Nmd Hostname set $NMD_HOSTNAME"
fi

if [ -z "$NETAGENT_CTRL_INTF" ]; then
    NETAGENT_CTRL_INTF="lo"
    echo "Net agent control intf not set, default to eth0"
else
    echo "Net agent control intf set to  $NETAGENT_CONTROL_INTF"
fi

if [ -z "$VENICE_IPS" ]; then
    echo "Venice IPs are not specified for agent."
else
    echo "Venice IPs specified for agent."
    IFS=', ' read -r -a array <<< $VENICE_IPS
    NPM_URL="-npm pen-npm"
    RESOLVER_URLS="-resolver-urls "
    for IP in "${array[@]}"
    do
        export CMD_URL="$IP"
        break
#        RESOLVER_URLS="$RESOLVER_URLS$IP:$CMD_RESOLVER_PORT "
    done
#    RESOLVER_URLS="${RESOLVER_URLS::-1}"
    MANAGED_MODE="-mode managed"
fi

if [ -z "$SMART_NIC_MODE" ]; then
    echo "Running naples-sim in host pinned mode"
else
    echo "Running naples-sim in smart nic mode"
    export SMART_NIC_MODE
fi

if [[ -z "${MAX_LIFS}" ]]; then
  MAX_LIFS=16
fi
export MAX_LIFS

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
    mkdir -p /naples/data/examples/config_tcp_proxy/
    mkdir -p /naples/data/examples/config_vxlan_overlay/
    cp /naples/nic/tools/bootstrap.sh /naples/data/
    cp /naples/nic/tools/bootstrap-naples.sh /naples/data/
    cp -r /naples/nic/data/examples/* /naples/data/examples/
    cp -r /naples/nic/data/examples/config_ipsec/* /naples/data/examples/config_ipsec/
    cp -r /naples/nic/data/examples/config_tcp_proxy/* /naples/data/examples/config_tcp_proxy/
    cp -r /naples/nic/data/examples/config_vxlan_overlay/* /naples/data/examples/config_vxlan_overlay/
fi

# starting the processes from log directory so that cores are saved there
cd "$LOG_DIR"

#$NIC_DIR/bin/cap_model +PLOG_MAX_QUIT_COUNT=0 +plog=info +model_debug=$HAL_CONFIG_PATH/iris/model_debug.json > $LOG_DIR/model.log 2>&1 &

if [ $WITH_QEMU == 1 ]; then
    echo "Running Capri model WITH Qemu"
    cd $PLATFORM_DIR/bin && sh setup_pcie.sh && cd - && SIMSOCK_PATH=/naples/data/simsock-turin LD_LIBRARY_PATH=$PLATFORM_DIR/lib:$LD_LIBRARY_PATH $PLATFORM_DIR/bin/model_server +PLOG_MAX_QUIT_COUNT=0 +plog=info -d type=eth,bdf=03:00.0,lif=2,intr_base=0,devcmd_pa=0x13809b000,devcmddb_pa=0x13809c000 > $LOG_DIR/model.log 2>&1 &
    PID=`ps -eaf | grep model_server | grep -v grep | awk '{print $2}'`
else
    echo "Running Capri model WITHOUT Qemu"
    $NIC_DIR/bin/cap_model +PLOG_MAX_QUIT_COUNT=0 > /dev/null 2>&1 &
    PID=`ps -eaf | grep cap_model | grep -v grep | awk '{print $2}'`
fi

if [[ "" ==  "$PID" ]]; then
    echo "Failed to start CAPRI model"
    #exit $?
else
    echo "CAPRI model started, pid is $PID"
fi

echo "Starting HAL ..."
$NIC_DIR/bin/hal -c hal.json > hal_run.log 2>&1 &
PID=`ps -eaf | grep hal | grep -v grep | awk '{print $2}'`
if [[ "" ==  "$PID" ]]; then
    echo "Failed to start HAL"
    #exit $?
else
    echo "HAL started, pid is $PID"
fi

# wait for HAL to open gRPC port before spawning agent(s)
HAL_WAIT_TIMEOUT=1
HAL_GRPC_PORT="${HAL_GRPC_PORT:-50054}"
HAL_SERVER="localhost/$HAL_GRPC_PORT"
HAL_UP=-1
MAX_RETRIES=600
i=0
until (( HAL_UP == 0 )) || (( i == MAX_RETRIES ))
do
    echo "Waiting for HAL GRPC server to be up ..."
    sleep "$HAL_WAIT_TIMEOUT"
    timeout 1 bash -c "cat < /dev/null > /dev/tcp/$HAL_SERVER"
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

if [ $WITH_QEMU == 1 ]; then
    echo "Starting real nicmgr...."
    LD_LIBRARY_PATH=$NICMGR_LIBRARY_PATH $PLATFORM_DIR/bin/nicmgrd -s -c $NICMGR_CONFIG_PATH/eth-smart.json > $LOG_DIR/nicmgr.log 2>&1 &
    [[ $? -ne 0 ]] && echo "Failed to start NICMGR!" && exit 1
    echo "NICMGR WAIT BEGIN: `date +%x_%H:%M:%S:%N`"
    while [ 1 ]
    do
        OUTPUT="$(grep  "Polling enabled" $LOG_DIR/nicmgr.log 2>&1)"
        if [[  ! -z "$OUTPUT" ]]; then
            break
        fi
        sleep 3
    done
    echo "NICMGR UP: `date +%x_%H:%M:%S:%N`"
else
# start NIC manager and allow it to create uplinks
    echo "Starting nicmgr ..."
    "$NIC_DIR"/bin/fake_nic_mgr
    if [ $? -ne 0 ]; then
        echo "Failed to start nic mgr"
        exit $?
    fi
fi

echo "Starting hntap ..."

if [ $WITH_QEMU == 1 ]; then
    echo "Starting hntap with Qemu mode"
    $NIC_DIR/bin/nic_infra_hntap -f $NIC_DIR/conf/hntap-with-qemu-cfg.json > $LOG_DIR/hntap.log 2>&1 &
else
    echo "Starting hntap in standalone mode"
    $NIC_DIR/bin/nic_infra_hntap -f $NIC_DIR/conf/hntap-cfg.json > $LOG_DIR/hntap.log 2>&1 &
fi

PID=`ps -eaf | grep nic_infra_hntap | grep -v grep | awk '{print $2}'`
if [[ "" ==  "$PID" ]]; then
    echo "Failed to start hntap service"
    #exit $?
else
    echo "hntap service started, pid is $PID"
fi

echo "Starting netagent ..."
"$NIC_DIR"/bin/netagent -logtofile $LOG_DIR/agent.log -resolver-urls "$CMD_URL":"$CMD_RESOLVER_PORT" $NPM_URL $MANAGED_MODE -datapath hal -disabletsa &

if [[ $NETAGENT_CTRL_INTF != "lo" ]]; then
    echo "Starting nmd ..."
    "$NIC_DIR"/bin/nmd  -cmdregistration "$CMD_URL":"$CMD_GRPC_UNAUTH_PORT" -cmdupdates "$CMD_URL":"$CMD_RESOLVER_PORT" -cmdcerts "$CMD_URL":"$CMD_RESOLVER_PORT" -hostif eth1 --log-to-file $LOG_DIR/nmd.log -resolver "$CMD_URL":"$CMD_RESOLVER_PORT" -mode network -hostname $NMD_HOSTNAME &
fi

echo "Starting delphi hub ..."
"$NIC_DIR"/bin/delphi_hub > $LOG_DIR/delphi_hub.log 2>&1 &

echo "NAPLES services/processes up and running ..."
# keep the container running
while :; do
      sleep 300
done
