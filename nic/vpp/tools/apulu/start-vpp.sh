#!/bin/sh

#VPP Partial init env variables
export NIC_DIR=/nic/
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export CONFIG_PATH=$NIC_DIR/conf/
export VPP_LOG_FILE=/var/log/pensando/vpp.log

ulimit -c unlimited

counter=600
start_vpp=0
output=""
while [ $counter -gt 0 ]
do
    output=$(cat /sys/class/uio/uio*/name | grep cpu_mnic0)
    if [ $output == "cpu_mnic0" ]; then
        start_vpp=1
        break
    fi
    sleep 1
    counter=$(( $counter - 1 ))
done
if [ $start_vpp == 0 ]; then
    echo "UIO device not created, not starting VPP!!"
    exit 1
fi
exec $NIC_DIR/bin/vpp -c $HAL_CONFIG_PATH/vpp/vpp_1_worker.conf
