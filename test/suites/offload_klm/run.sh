#!/bin/bash
cd /tmp/git/pensando/sw/storage/offload && make modules && sudo insmod pnso.ko && dmesg | grep "PNSO: Osal test complete" && sudo rmmod pnso && dmesg > /tmp/ClusterLogs
