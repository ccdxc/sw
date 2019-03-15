#! /bin/bash
export SWDIR=`git rev-parse --show-toplevel`
export NICDIR=$SWDIR/nic/
export CAPRI_MOCK_MODE=1
export CAPRI_MOCK_MEMORY_MODE=1
export BLDDIR=${NICDIR}/build/x86_64/apollo
export COVFILE=${NICDIR}/coverage/sim_bullseye_hal.cov

rm -f ${NICDIR}/core.*

if [[ "$1" ==  --coveragerun ]]; then
    CMD_OPTS="COVFILE\=${COVFILE}"
fi

MEMHASH_PRELOADS=${BUILD_DIR}/lib/libmemhashp4pd_mock.so
set -e
# PI gtests
export PATH=${PATH}:${BLDDIR}/bin
LD_PRELOAD=${MEMHASH_PRELOADS} $ARGS memhash_test $*
