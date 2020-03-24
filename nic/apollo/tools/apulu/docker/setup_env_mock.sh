#! /bin/bash

if [ -z "$1" ]
then
    echo "Pipeline argument not supplied"
    exit 1
fi

export PDSPKG_TOPDIR=$NIC_DIR/
export PIPELINE=$1
export BUILD_DIR=${PDSPKG_TOPDIR}/build/x86_64/${PIPELINE}/
export CONFIG_PATH=$PDSPKG_TOPDIR/conf/
export PATH=${PATH}:${BUILD_DIR}/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PDSPKG_TOPDIR/third-party/metaswitch/output/x86_64/:$PDSPKG_TOPDIR/lib:$PDSPKG_TOPDIR/lib64:$NIC_DIR/../usr/lib
export PERSISTENT_LOGDIR=$PDSPKG_TOPDIR
export NON_PERSISTENT_LOGDIR=$PDSPKG_TOPDIR
export LOG_DIR=$NON_PERSISTENT_LOGDIR

export ZMQ_SOC_DIR=$PDSPKG_TOPDIR

export ASIC_MOCK_MODE=1
export ASIC_MOCK_MEMORY_MODE=1
export VPP_IPC_MOCK_MODE=1

#GDB='gdb --args'
#VALGRIND='valgrind --leak-check=full --show-leak-kinds=all --gen-suppressions=all --error-limit=no --verbose --log-file=valgrind-out.txt --track-origins=yes'

