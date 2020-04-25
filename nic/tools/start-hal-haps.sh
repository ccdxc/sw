#!/bin/sh

SYSCONFIG=/sysconfig/config0
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
export NIC_DIR=`dirname $ABS_TOOLS_DIR`
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export PERSISTENT_LOGDIR=/obfl
export NON_PERSISTENT_LOGDIR=/var/log/pensando
export HAL_LIBRARY_PATH=$NIC_DIR/lib:$NIC_DIR/../platform/lib:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
export HAL_PBC_INIT_CONFIG="2x100_hbm"
export COVFILE=$HAL_CONFIG_PATH/hw_bullseye_hal.cov
#export DISABLE_AGING=1

#nicmgr specific
export PLATFORM_DIR='/platform'
export LOG_DIR='/obfl'
export NICMGR_LOG_LEVEL=debug
export NICMGR_CONFIG_PATH=$PLATFORM_DIR/etc/nicmgrd
export NICMGR_LIBRARY_PATH=$PLATFORM_DIR/lib:$NIC_DIR/lib:$NIC_DIR/conf/sdk
export HAL_LIBRARY_PATH=$HAL_LIBRARY_PATH:$NICMGR_LIBRARY_PATH

# remove logs
rm -f $NON_PERSISTENT_LOGDIR/hal.log*

ulimit -c unlimited

export SYSUUID=""
if [ -r $SYSCONFIG/sysuuid ]; then
    export SYSUUID=`cat $SYSCONFIG/sysuuid`
fi

if [[ "$FWD_MODE" == "classic" ]]; then
    cp $HAL_CONFIG_PATH/hal_classic.ini $HAL_CONFIG_PATH/hal.ini
elif [[ "$FWD_MODE" == "hostpin" ]]; then
    cp $HAL_CONFIG_PATH/hal_hostpin.ini $HAL_CONFIG_PATH/hal.ini
elif [[ "$FWD_MODE" == "switch" ]]; then
    cp $HAL_CONFIG_PATH/hal_switch.ini $HAL_CONFIG_PATH/hal.ini
else
    echo "Unknown forwarding mode $FWD_MODE"
    exit 1
fi

if [[ "$PLATFORM" != "hw" ]]; then
    export LD_LIBRARY_PATH=$HAL_LIBRARY_PATH
    exec $NIC_DIR/bin/hal -c hal_haps.json
else
    export LD_LIBRARY_PATH=$HAL_LIBRARY_PATH
    exec $NIC_DIR/bin/hal -c hal_hw.json
fi
[[ $? -ne 0 ]] && echo "Failed to start HAL!" && exit 1

# echo "HAL WAIT BEGIN: `date +%x_%H:%M:%S:%N`"

# while [ 1 ]
# do
#     OUTPUT="$(cat /hal.log 2>&1 | grep "gRPC server listening on")"
#     if [[ ! -z "$OUTPUT" ]]; then
#         break
#     fi
#     sleep 3
# done

# echo "HAL UP: `date +%x_%H:%M:%S:%N`"
