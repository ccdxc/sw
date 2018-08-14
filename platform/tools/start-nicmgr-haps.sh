
export LD_LIBRARY_PATH=/platform/lib:/nic/lib:/nic/conf/sdk:$LD_LIBRARY_PATH

ulimit -c unlimited

/platform/bin/nicmgrd > nicmgr.log 2>&1
