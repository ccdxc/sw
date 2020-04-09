#! /bin/bash -e

TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
export ASIC="${ASIC:-capri}"
export NICDIR=`dirname $ABS_TOOLS_DIR`/../..
export DOLDIR=`readlink -f ${NICDIR}/../dol/`

rm -f ${NICDIR}/nic_sanity_logs.tar.gz

if ls ${NICDIR}/core.* 1> /dev/null 2>&1; then
    echo "waiting for the core to be produced"
    sleep 60
fi

set -ex
tar cvzf ${NICDIR}/nic_sanity_logs.tar.gz -P --ignore-failed-read \
    ${NICDIR}/core.* \
    ${NICDIR}/*log* \
    ${DOLDIR}/*log* \
    ${NICDIR}/conf/gen/mpu_prog_info.json \
    ${NICDIR}/build/x86_64/apollo/${ASIC}/gtest_results/valgrind_apollo_scale_test_1vpc.log \
    ${NICDIR}/build/x86_64/artemis/${ASIC}/gtest_results/valgrind_apollo_scale_test_1vpc.log \
    *.log.*
