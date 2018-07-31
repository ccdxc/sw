export HAL_CONFIG_PATH=/nic/conf/
export LD_LIBRARY_PATH=/nic/lib:/nic/conf/sdk:/nic/conf/sdk/external:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
ulimit -c unlimited
$GDB /nic/bin/linkmgr 2>&1
