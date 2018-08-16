#!/usr/bin/env bash
BRINGUPNAPLES=$1

export NIC_DIR='/nic'
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LD_LIBRARY_PATH=$NIC_DIR/lib:$NIC_DIR/conf/sdk:$NIC_DIR/conf/linkmgr:$NIC_DIR/conf/sdk/external:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
export HAL_PBC_INIT_CONFIG="2x100_hbm"
ulimit -c unlimited

# Bring up nig mgr and agent
$GDB $NIC_DIR/bin/hal -c hal_haps.json

if [ -n "$BRINGUPNAPLES" ]; then
  # Wait for HAL
  wait-for-hal

  # Run NIC mgr
  $NIC_DIR/bin/nic_mgr_app

  # Start Netagent in background
  $NIC_DIR/bin/netagent -datapath hal -logtofile /tmp/agent.log -hostif lo &
fi


function wait-for-hal()
{
  # MAX_RETRIES guards the maximum time we wait for HAL to come up. It waits for 2**(MAX_RETRIES) - 1 seconds
  MAX_RETRIES=10
  HAL_GRPC_PORT="${HAL_GRPC_PORT:-50054}"
  HAL_SERVER="localhost:$HAL_GRPC_PORT"
  HAL_UP=-1

  echo "HAL WAIT BEGIN: `date +%x_%H:%M:%S:%N`"

  until (( HAL_UP == 0 )) || (( i == MAX_RETRIES ))
  do
  	timeout="$((2 ** i))"
  	echo "Waiting for HAL GRPC server to be up. Sleeping for $timeout seconds..."
  	sleep "$timeout"
  	curl "$HAL_SERVER"
  	HAL_UP="$?"
  	let "i++"
  done
  if [ $i -eq $MAX_RETRIES ]; then
  	echo "HAL server failed to come up"
  	exit 1
  fi
  echo "HAL WAIT END: `date +%x_%H:%M:%S:%N`"
}
