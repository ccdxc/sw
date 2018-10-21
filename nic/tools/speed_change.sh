#!/bin/bash
sim=
NIC_DIR=/nic

# speed change to 10G
# /nic/tools/speed_change.sh <1|5> 10

if [[ "$#" != "2" ]]; then
    echo "Usage: $0 <port> <new_speed>"
    exit 1
fi

port=$1
speed=$2

if [[ $port != "1" ]] && [[ $port != "5" ]]; then
    echo "Port can only be 1 or 5"
    exit 1
fi

echo "Port: $port, New speed: $speed"

p=$port
$NIC_DIR/tools/port_op.sh $sim --port $p --delete
p=$((p + 1))
$NIC_DIR/tools/port_op.sh $sim --port $p --delete
p=$((p + 1))
$NIC_DIR/tools/port_op.sh $sim --port $p --delete
p=$((p + 1))
$NIC_DIR/tools/port_op.sh $sim --port $p --delete

if [[ $speed == "100" ]] || [[ $speed == "40" ]]; then
    $NIC_DIR/tools/port_op.sh $sim --port $port --create --speed $speed --enable 1
else
    p=$port
    $NIC_DIR/tools/port_op.sh $sim --port $p --create --speed $speed --enable 1
    p=$((p + 1))
    $NIC_DIR/tools/port_op.sh $sim --port $p --create --speed $speed --enable 1
    p=$((p + 1))
    $NIC_DIR/tools/port_op.sh $sim --port $p --create --speed $speed --enable 1
    p=$((p + 1))
    $NIC_DIR/tools/port_op.sh $sim --port $p --create --speed $speed --enable 1
fi
