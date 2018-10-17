#!/bin/bash

# env
export NIC_DIR='/nic'
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LD_LIBRARY_PATH=$NIC_DIR/lib:$NIC_DIR/conf/sdk:$NIC_DIR/conf/linkmgr:$NIC_DIR/conf/sdk/external:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH

ulimit -c unlimited

set -o errexit -o pipefail -o noclobber -o nounset

PORT_CLIENT=$NIC_DIR/bin/port_client
GRPC_PORT=localhost:50054
GDB=

OPTIONS=crudp:s:y:e:ith
LONGOPTS=create,get,update,delete,port:,speed:,type:,enable:,sim,test,help

! PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTS --name "$0" -- "$@")
if [[ ${PIPESTATUS[0]} -ne 0 ]]; then
    exit 2
fi

eval set -- "$PARSED"

create=0
get=0
update=0
delete=0
port=0
speed=0
en=0
dry_run=0
port_type=eth

while true; do
    case "$1" in
        -c|--create)
            create=1
            shift
            ;;
        -r|--get)
            get=1
            shift
            ;;
        -u|--update)
            update=1
            shift
            ;;
        -d|--delete)
            delete=1
            shift
            ;;
        -p|--port)
            port=$2
            shift 2
            ;;
        -s|--speed)
            speed=$2
            shift 2
            ;;
        -y|--type)
            port_type=$2
            shift 2
            ;;
        -e|--enable)
            en=$2
            shift 2
            ;;
        -i|--sim)
            PORT_CLIENT=./build/x86_64/iris/bin/port_client
            shift
            ;;
        -t|--test)
            dry_run=1
            shift
            ;;
        -h|--help)
            echo "Usage:"
            echo "$0 [--sim] --create --port <port_num> --speed <100|40|50|25|10> --type <eth/mgmt> --enable <0|1>"
            echo "$0 [--sim] --get    --port <port_num> "
            echo "$0 [--sim] --update --port <port_num> --enable <0|1>"
            echo "$0 [--sim] --delete --port <port_num>"
            exit 0
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "error"
            exit 3
            ;;
    esac
done

num_lanes=1
fec=none
mac_id=0
mac_ch=0
admin_st=down

if [[ "$dry_run" == "1" ]]; then
    dry_run="-t"
else
    dry_run=
fi

if [[ "$create" == "1" ]]; then
    if [[ "$speed" == "100" ]]; then
        num_lanes=4
        fec=fc
    elif [[ "$speed" == "50" ]]; then
        num_lanes=2
        fec=fc
    elif [[ "$speed" == "40" ]]; then
        num_lanes=4
        fec=none
    elif [[ "$speed" == "25" ]]; then
        num_lanes=1
        fec=none
    fi

    if [[ $port == "5" ]]; then
        mac_id=1
    fi

    if [[ "$en" == "1" ]];then
        admin_st=up
    fi

    CMD="$GDB $PORT_CLIENT -g $GRPC_PORT --create -p $port --speed $speed --num_lanes $num_lanes --port_type $port_type --fec_type $fec --mac_id $mac_id --mac_ch $mac_ch --admin_state $admin_st $dry_run"
fi

if [[ "$get" == "1" ]]; then
    if [[ "$port" == "0" ]];then
        CMD="$GDB $PORT_CLIENT -g $GRPC_PORT --get $dry_run"
    else
        CMD="$GDB $PORT_CLIENT -g $GRPC_PORT --get -p $port $dry_run"
    fi
fi

if [[ "$update" == "1" ]]; then
    if [[ "$en" == "1" ]];then
        admin_st=up
    fi
    CMD="$GDB $PORT_CLIENT -g $GRPC_PORT --update -p $port --admin_state $admin_st $dry_run"
fi

if [[ "$delete" == "1" ]]; then
    CMD="$GDB $PORT_CLIENT -g $GRPC_PORT --delete -p $port $dry_run"
fi

echo $CMD
$CMD
