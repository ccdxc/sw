#! /bin/bash
export ASIC="${ASIC:-capri}"
export SWDIR=`git rev-parse --show-toplevel`
export NICDIR=$SWDIR/nic/
export ASIC_MOCK_MODE=1
export ASIC_MOCK_MEMORY_MODE=1
export COVFILE=${NICDIR}/coverage/sim_bullseye_hal.cov

rm -f ${NICDIR}/core.*

if [[ "$1" ==  --coveragerun ]]; then
    CMD_OPTS="COVFILE\=${COVFILE}"
fi

set -e
# PI gtests
export PATH=${PATH}:${NICDIR}/build/x86_64/artemis/${ASIC}/bin
echo "Starting " `which ftl_test`
sleep 1
$ARGS ftl_test $*
#perf record --call-graph fp ftl_test --gtest_filter="scale.insert1M"
#gdb --args ftl_test --gtest_filter="basic.insert_update_remove"
