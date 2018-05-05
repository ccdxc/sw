#! /bin/bash

set -e

# check agent's health
curl localhost:9007/api/tenants/
if [ $? -ne 0 ]; then
    echo "Agent not running"
    exit 1
fi

# check HAL's health
HAL_GRPC_PORT="${HAL_GRPC_PORT:-50054}"
HAL_SERVER="localhost:$HAL_GRPC_PORT"
curl "$HAL_SERVER"
if [ $? -ne 0 ]; then
    echo "HAL not running"
    exit 1
fi

# check CAPRI's health
PID=`ps -eaf | grep cap_model | grep -v grep | awk '{print $2}'`
if [[ "" ==  "$PID" ]]; then
    echo "CAPRI model not running"
    exit 1
fi

# check hntap's health
PID=`ps -eaf | grep nic_infra_hntap | grep -v grep | awk '{print $2}'`
if [[ "" ==  "$PID" ]]; then
    echo "hntap service model not running"
    exit 1
fi

echo "NAPLES health - ok"
exit 0
