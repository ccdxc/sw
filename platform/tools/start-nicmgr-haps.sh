#!/bin/sh

SYSCONFIG=/sysconfig/config0

export NIC_DIR='/nic'
export PLATFORM_DIR='/platform'
export NIC_LOG_DIR=/var/log
export NICMGR_LOG_LEVEL=debug
export NICMGR_CONFIG_PATH=$PLATFORM_DIR/etc/nicmgrd
export NICMGR_LIBRARY_PATH=$PLATFORM_DIR/lib:$NIC_DIR/lib:$NIC_DIR/conf/sdk:$LD_LIBRARY_PATH
export HAL_CONFIG_PATH=$NIC_DIR/conf/

# Thie must be set before the script gets executed
# export FWD_MODE="$1"

# Remove logs
rm -f /nicmgr.log*

ulimit -c unlimited

export SYSUUID=""
if [ -r $SYSCONFIG/sysuuid ]; then
    export SYSUUID=`cat $SYSCONFIG/sysuuid`
fi

#sleep 30

if [[ "$FWD_MODE" != "classic" ]]; then
    if [ -r $SYSCONFIG/eth-smart.json ]; then
        ARGS="-s -c $SYSCONFIG/eth-smart.json -p haps"
    else
        ARGS="-s -c $NICMGR_CONFIG_PATH/eth-smart.json -p haps"
    fi
else
    if [ -r $SYSCONFIG/device.json ]; then
        ARGS="-c $SYSCONFIG/device.json -p haps"
    else
        ARGS="-c $NICMGR_CONFIG_PATH/device.json -p haps"
    fi
fi

export LD_LIBRARY_PATH=$NICMGR_LIBRARY_PATH
exec $PLATFORM_DIR/bin/nicmgrd $ARGS
# [[ $? -ne 0 ]] && echo "Failed to start NICMGR!" && exit 1

# echo "NICMGR WAIT BEGIN: `date +%x_%H:%M:%S:%N`"

# while [ 1 ]
# do
#     OUTPUT="$(tail -100 /nicmgr.stdout 2>&1 | grep "Polling enabled")"
#     if [[  ! -z "$OUTPUT" ]]; then
#         break
#     fi
#     sleep 3
# done

# echo "NICMGR UP: `date +%x_%H:%M:%S:%N`"
