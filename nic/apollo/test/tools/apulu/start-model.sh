#!/bin/bash -e

export ASIC="${ASIC:-capri}"
CUR_DIR=$( readlink -f $( dirname $0 ) )
source $CUR_DIR/../../../tools/setup_env_sim.sh apulu

${PDSPKG_TOPDIR}/tools/merge_model_debug.py --pipeline apulu --p4 apulu --rxdma p4plus_rxdma --txdma p4plus_txdma
$GDB $PDSPKG_TOPDIR/build/x86_64/apulu/${ASIC}/bin/cap_model \
    +PLOG_MAX_QUIT_COUNT=0 +plog=info \
    +model_debug=$PDSPKG_TOPDIR/build/x86_64/apulu/${ASIC}/gen/p4gen//apulu/dbg_out/combined_model_debug.json 2>&1 > $PDSPKG_TOPDIR/model.log
