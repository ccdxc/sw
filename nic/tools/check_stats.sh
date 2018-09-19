# env
export NIC_DIR='/nic'
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LD_LIBRARY_PATH=$NIC_DIR/lib:$NIC_DIR/conf/sdk:$NIC_DIR/conf/linkmgr:$NIC_DIR/conf/sdk/external:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH

kill -STOP `pidof hal`

/nic/bin/port_client -g localhost:50053 -o 18 -w $1 -x 0

# flush linkmgr logs
kill -10 `pidof linkmgr`
tail -80 /run.log | grep -E "Frames Transmitted (OK|All)"
tail -80 /run.log | grep -E "Frames Received (OK|All)"

kill -CONT `pidof hal`