#! /bin/bash

export PDSPKG_TOPDIR='/nic'
export SYSCONFIG='/sysconfig/config0'
export LD_LIBRARY_PATH=$PDSPKG_TOPDIR/lib:/usr/local/lib:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH
export CONFIG_PATH=$PDSPKG_TOPDIR/conf/
export COVFILE=$PDSPKG_TOPDIR/coverage/sim_bullseye_hal.cov
export HAL_PBC_INIT_CONFIG="2x100_hbm"
export PERSISTENT_LOGDIR='/obfl/'
export NON_PERSISTENT_LOGDIR='/var/log/pensando/'
export LOG_DIR=$NON_PERSISTENT_LOGDIR

#GDB='gdb --args'
#VALGRIND='valgrind --leak-check=full --show-leak-kinds=all --gen-suppressions=all --error-limit=no --verbose --log-file=valgrind-out.txt --track-origins=yes'
