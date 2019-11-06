#! /bin/bash

set -e
export TOOLDIR=`dirname $0`
export SKIP_VERIFY=1
export SDKDIR=`readlink -f $TOOLDIR/..`
export BUILD_DIR=${SDKDIR}/build/x86_64/
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results
export PATH=${PATH}:${BUILD_DIR}/bin
#export GDB='gdb --args'


if [[ "$1" ==  --coveragerun ]]; then
    # Coverage will be running as part of the pipeline tests
    if [ -z "${COVFILE}" ];then
        echo "Coverge file not set"
        exit 1
    fi
fi

function finish {
    echo "===== Collecting logs ====="
    ${SDKDIR}/tools/savelogs.sh
}

trap finish EXIT

# gtests
echo "Running memhash test"
${SDKDIR}/lib/table/memhash/test/run.sh

echo "Running slhash test"
${SDKDIR}/lib/table/slhash/test/run.sh

echo "Running sltcam test"
${SDKDIR}/lib/table/sltcam/test/run.sh

echo "Running rte_indexer test"
$GDB ${BUILD_DIR}/bin/rte_indexer_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/rte_indexer_test.xml" > ${SDKDIR}/rte_indexer_test.log.txt
[[ $? -ne 0 ]] && echo "rte_indexer_test failed!" && exit 1

echo "Running indexer test"
$GDB ${BUILD_DIR}/bin/indexer_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/indexer_test.xml" > ${SDKDIR}/indexer_test.log.txt
[[ $? -ne 0 ]] && echo "indexer_test failed!" && exit 1

echo "Running sldirectmap test"
$GDB ${BUILD_DIR}/bin/sldirectmap_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/sldirectmap_test.xml" > ${SDKDIR}/sldirectmap_test.log.txt
[[ $? -ne 0 ]] && echo "sldirectmap_test failed!" && exit 1

echo "Success"
