#!/bin/bash -e

TOP_DIR=`git rev-parse --show-toplevel`
export ASIC="${ASIC:-capri}"
export NIC_DIR="$TOP_DIR/nic"
export ZMQ_SOC_DIR=${NIC_DIR}
${NIC_DIR}/tools/merge_model_debug.py --pipeline athena --p4 athena --rxdma p4plus_rxdma --txdma p4plus_txdma
$GDB $NIC_DIR/build/x86_64/athena/${ASIC}/bin/cap_model \
    +PLOG_MAX_QUIT_COUNT=0 +plog=info \
    +model_debug=$NIC_DIR/build/x86_64/athena/${ASIC}/gen/p4gen//athena/dbg_out/combined_model_debug.json 2>&1 > $NIC_DIR/model.log
