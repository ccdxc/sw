# env
export NIC_DIR='/nic'
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LD_LIBRARY_PATH=$NIC_DIR/lib:$NIC_DIR/conf/sdk:$NIC_DIR/conf/linkmgr:$NIC_DIR/conf/sdk/external:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH

check_link() {
    up=`$NIC_DIR/tools/port_op.sh --get --port $1 | grep oper | grep -o '[0-9]*'`
    if [[ "$up" == "1" ]]; then
        echo "Port $1 is up"
    else
        echo "Port $1 is down"
    fi
}

check_link 1
check_link 5

exit 0

kill -STOP `pidof hal`

/nic/bin/port_client -g localhost:50053 -o 6 -w 0
/nic/bin/port_client -g localhost:50053 -o 6 -w 4

# flush linkmgr logs
kill -10 `pidof linkmgr`
tail /linkmgr.log | grep "sync:"
kill -CONT `pidof hal`
