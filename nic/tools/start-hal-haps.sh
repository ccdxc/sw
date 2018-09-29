#!/bin/sh

TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
export NIC_DIR=`dirname $ABS_TOOLS_DIR`
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export HAL_LIBRARY_PATH=$NIC_DIR/lib:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
export HAL_PBC_INIT_CONFIG="2x100_hbm"
export HAL_LOG_FILE='/hal.log'
export FWD_MODE="$1"
export PLATFORM="$2"
export DISABLE_AGING=1

# Remove logs
rm -f /hal.log*

ulimit -c unlimited

if [[ "$FWD_MODE" != "classic" ]]; then
    cp $HAL_CONFIG_PATH/hal_hostpin.ini $HAL_CONFIG_PATH/hal.ini
else
    cp $HAL_CONFIG_PATH/hal_classic.ini $HAL_CONFIG_PATH/hal.ini
fi

if [[ "$PLATFORM" != "hw" ]]; then
    LD_LIBRARY_PATH=$HAL_LIBRARY_PATH $NIC_DIR/bin/hal -c hal_haps.json > $HAL_LOG_FILE 2>&1 &
else
    LD_LIBRARY_PATH=$HAL_LIBRARY_PATH $NIC_DIR/bin/hal -c hal_hw.json -p catalog_100g.json > $HAL_LOG_FILE 2>&1 &
fi
[[ $? -ne 0 ]] && echo "Failed to start HAL!" && exit 1

echo "HAL WAIT BEGIN: `date +%x_%H:%M:%S:%N`"

while [ 1 ]
do
    OUTPUT="$(tail $HAL_LOG_FILE 2>&1 | grep "gRPC server listening on")"
    if [[ ! -z "$OUTPUT" ]]; then
        break
    fi
    sleep 3
done

echo "HAL UP: `date +%x_%H:%M:%S:%N`"
