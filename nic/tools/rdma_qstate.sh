#!/bin/bash
#
# This script gathers RDMA queue state info from halctl and eth_dbgtool.
# It is intended to be run on the DSC.
#
LIFS=`/nic/bin/halctl show lif | grep RE | grep -v Flags | cut -d " " -f 1`

for LIF in $LIFS
do
	echo === RDMA LIF $LIF start ===
	/platform/bin/eth_dbgtool rdma_qstate $LIF
	echo ===  RDMA LIF $LIF end  ===
done
