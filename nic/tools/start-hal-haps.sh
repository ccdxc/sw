export HAL_CONFIG_PATH=/nic/conf/
export LD_LIBRARY_PATH=/nic/lib:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
export HAL_PBC_INIT_CONFIG="2x100_hbm"
ulimit -c unlimited
$GDB /nic/bin/hal -c hal_haps.json > hal.log 2>&1
