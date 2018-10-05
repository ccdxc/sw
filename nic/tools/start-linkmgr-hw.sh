# env
export NIC_DIR='/nic'
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LD_LIBRARY_PATH=$NIC_DIR/lib:$NIC_DIR/conf/sdk:$NIC_DIR/../platform/lib:$NIC_DIR/conf/linkmgr:$NIC_DIR/conf/sdk/external:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH

# start linkmgr
ulimit -c unlimited
# start linkmgr
$GDB $NIC_DIR/bin/linkmgr -c linkmgr_hw.json -p catalog_hw.json 2>&1 > /run.log &

sleep 10

# start AACS server
kill -STOP `pidof hal`
$NIC_DIR/bin/port_client -g localhost:50053 -o 26 -w 9000
kill -CONT `pidof hal`
