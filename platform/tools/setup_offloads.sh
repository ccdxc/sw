#!/bin/bash


IFS=()

for if_file in /sys/class/net/*; do
	if_dev=$(basename "$if_file")
	x=`ethtool -i $if_dev 2>&1 | grep 'ionic'`
	if [[ -n "$x" ]]; then
		IFS+=($if_dev)
	fi
done

echo "Ionic Ifs: ${IFS[@]}"

set -x

for IF in ${IFS[@]}; do
    # Configure interrupt coalescing to reduce interrupt load
    # ethtool -C $IF rx-usecs 64 tx-usecs 128

    ethtool -K $IF rx off tx off txvlan off rxvlan off sg off tso off gro off gso off rxhash off
    ethtool -K $IF txvlan on rxvlan on

    ethtool -K $IF rx on tx on

    ethtool -K $IF sg on
    ethtool -K $IF gso on
    ethtool -K $IF tso on
    ethtool -K $IF gro on
    ethtool -K $IF rxhash on
done

# Disable services that can interfere with performance tuning
service irqbalance stop
#service tuned stop
#service numad stop
ufw disable

# Enable fair queueing schedule for TCP (improves pacing)
#tc qdisc add dev enp96s0 root fq

# Reduce power saving
cpufreq-set -r -g performance

# Allow 2GB network buffers (Max allowed by Linux)
sysctl -w net.core.rmem_max=2147483647
sysctl -w net.core.wmem_max=2147483647

# Allow auto-tuning of TCP buffers upto 2GB (Max allowed by Linux)
# Formula: Tput * RTT
sysctl -w net.ipv4.tcp_rmem="4096 87380 2147483647"
sysctl -w net.ipv4.tcp_wmem="4096 65536 2147483647"

