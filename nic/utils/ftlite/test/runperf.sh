#! /bin/bash
export SWDIR=`git rev-parse --show-toplevel`
export NICDIR=$SWDIR/nic/
export ASIC_MOCK_MODE=1
export ASIC_MOCK_MEMORY_MODE=1
export BLDDIR=${NICDIR}/build/x86_64/apollo
export COVFILE=${NICDIR}/coverage/sim_bullseye_hal.cov

rm -f ${NICDIR}/core.*

if [[ "$1" ==  --coveragerun ]]; then
    CMD_OPTS="COVFILE\=${COVFILE}"
fi

set -e
# PI gtests
export PATH=${PATH}:${BLDDIR}/bin
#perf record --call-graph fp ftlite_test --gtest_filter="*"
perf stat -e cycles,instructions,cache-references,cache-misses,bus-cycles,L1-dcache-loads,L1-dcache-load-misses,L1-dcache-stores ftlite_test
