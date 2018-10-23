#!/bin/sh

export NIC_DIR='/nic'
export PLATFORM_DIR='/platform'
export NICMGR_CONFIG_PATH=$PLATFORM_DIR/etc/nicmgrd
export NICMGR_LIBRARY_PATH=$PLATFORM_DIR/lib:$NIC_DIR/lib:$NIC_DIR/conf/sdk:$LD_LIBRARY_PATH
export NICMGR_LOG_FILE='/nicmgr.log'
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export FWD_MODE="$1"

# Remove logs
rm -f /nicmgr.log*

ulimit -c unlimited

if [[ "$FWD_MODE" != "classic" ]]; then
    ARGS="-s -c $NICMGR_CONFIG_PATH/eth-smart.json -p haps"
else
    ARGS="-c $NICMGR_CONFIG_PATH/device.json -p haps"
fi

LD_LIBRARY_PATH=$NICMGR_LIBRARY_PATH $PLATFORM_DIR/bin/nicmgrd $ARGS > /nicmgr.stdout 2>&1 &
[[ $? -ne 0 ]] && echo "Failed to start NICMGR!" && exit 1

echo "NICMGR WAIT BEGIN: `date +%x_%H:%M:%S:%N`"

while [ 1 ]
do
    OUTPUT="$(tail -100 /nicmgr.stdout 2>&1 | grep "Polling enabled")"
    if [[  ! -z "$OUTPUT" ]]; then
        break
    fi
    sleep 3
done

echo "NICMGR UP: `date +%x_%H:%M:%S:%N`"
