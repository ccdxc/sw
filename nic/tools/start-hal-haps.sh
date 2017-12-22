export HAL_CONFIG_PATH=/nic/conf/
export LD_LIBRARY_PATH=/nic/lib:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
$GDB /nic/bin/hal -c hal.json 2>&1 | tee hal.log
