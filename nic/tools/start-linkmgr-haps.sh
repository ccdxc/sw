export HAL_CONFIG_PATH=/nic/conf/
export LD_LIBRARY_PATH=/nic/conf/libs/sdk:/nic/lib:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
$GDB /nic/bin/linkmgr 2>&1 | tee linkmgr.log
