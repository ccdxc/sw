#! /bin/bash
export SWDIR=`git rev-parse --show-toplevel`
export NICDIR=$SWDIR/nic/
export CAPRI_MOCK_MODE=1
export CAPRI_MOCK_MEMORY_MODE=1
export COVFILE=${NICDIR}/coverage/sim_bullseye_hal.cov

rm -f ${NICDIR}/core.*

if [[ "$1" ==  --coveragerun ]]; then
    CMD_OPTS="COVFILE\=${COVFILE}"
fi

set -e
# PI gtests
export PATH=${PATH}:${NICDIR}/build/x86_64/iris/bin
echo "Starting " `which iris_ftl_test`
sleep 1
$ARGS iris_ftl_test --threads=4 $*
valgrind --track-origins=yes --leak-check=full --show-leak-kinds=definite iris_ftl_test --gtest_filter=basic*:collision* --log-file=ftl_test_valgrind.log
#perf record --call-graph fp ftl_test --gtest_filter="scale.insert1M"
#gdb --args ftl_test --gtest_filter="basic.insert"
