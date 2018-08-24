#!/bin/sh

export NIC_DIR='/nic'
export PLATFORM_DIR='/platform'
export FWD_MODE="$1"

ulimit -c unlimited

# check for all the binaries
if [[ ! -f $NIC_DIR/bin/hal ]]; then
    echo "HAL binary not found"
    exit 1
fi

if [[ ! -f $NIC_DIR/bin/netagent ]]; then
    echo "netagent binary not found"
    exit 1
fi

if [[ ! -f $PLATFORM_DIR/bin/nicmgrd ]]; then
    echo "nicmgr binary not found"
    exit 1
fi

# start HAL
$NIC_DIR/tools/start-hal-haps.sh "$FWD_MODE"
[[ $? -ne 0 ]] && echo "Aborting Sysinit - HAL failed to start!" && exit 1

# start nicmgr
$PLATFORM_DIR/tools/start-nicmgr-haps.sh "$FWD_MODE"
[[ $? -ne 0 ]] && echo "Aborting Sysinit - NICMGR failed to start!" && exit 1

# start netagent
if [[ "$FWD_MODE" != "classic" ]]; then
    # Remove logs
    rm -f agent.log*

    $NIC_DIR/bin/netagent -datapath hal -logtofile /agent.log -hostif lo &
    [[ $? -ne 0 ]] && echo "Failed to start AGENT!" && exit 1
fi

echo "All processes brought up, please check ..."
