#! /bin/bash

if [ -z "$1" ]
then
    echo "Pipeline argument not supplied"
    exit 1
fi

export PDSPKG_TOPDIR='/sw/nic/'
export PIPELINE=$1
export BUILD_DIR=${PDSPKG_TOPDIR}/build/x86_64/${PIPELINE}/
export CONFIG_PATH=$PDSPKG_TOPDIR/conf/
export COVFILE=$PDSPKG_TOPDIR/coverage/sim_bullseye_hal.cov
export PATH=${PATH}:${BUILD_DIR}/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PDSPKG_TOPDIR/third-party/metaswitch/output/x86_64/
export PERSISTENT_LOGDIR=$PDSPKG_TOPDIR
export NON_PERSISTENT_LOGDIR=$PDSPKG_TOPDIR
export LOG_DIR=$NON_PERSISTENT_LOGDIR

export CAPRI_MOCK_MODE=1
export CAPRI_MOCK_MEMORY_MODE=1
if [ $PIPELINE != 'apulu' ]; then
    export VPP_IPC_MOCK_MODE=1
fi

#GDB='gdb --args'
#VALGRIND='valgrind --leak-check=full --show-leak-kinds=all --gen-suppressions=all --error-limit=no --verbose --log-file=valgrind-out.txt --track-origins=yes'
