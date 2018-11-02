#!/bin/bash

NUM_CPUS=${1:-$(ls /sys/bus/cpu/devices | wc -l)}
GOVERNOR=${2:-performance}

for ((CPU=0; CPU < NUM_CPUS; ++CPU)) ; do
	cpufreq-set -c "$CPU" -g "$GOVERNOR"
done
