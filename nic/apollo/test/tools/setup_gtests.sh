#! /bin/bash

set -e

export SDKDIR=${NICDIR}/sdk/
export NON_PERSISTENT_LOGDIR=${NICDIR}
export ZMQ_SOC_DIR=${NICDIR}
export CAPRI_MOCK_MODE=1
export CAPRI_MOCK_MEMORY_MODE=1
export SKIP_VERIFY=1
export BUILD_DIR=${NICDIR}/build/x86_64/${PIPELINE}/
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results
export HAL_CONFIG_PATH=${NICDIR}/conf
export COVFILE=${NICDIR}/coverage/sim_bullseye_hal.cov
export PATH=${PATH}:${BUILD_DIR}/bin
export VAL_CMD=valgrind
#export GDB='gdb --args'

function finish () {
    # auto invoked on any exit
    echo "===== Collecting logs ====="
    ${NICDIR}/apollo/test/tools/savelogs.sh
    rm -f ${NICDIR}/conf/pipeline.json
}
trap finish EXIT

function setup () {
    rm -f ${NICDIR}/*log* ${NICDIR}/core*
    rm -f ${NICDIR}/conf/pipeline.json
    ln -s ${NICDIR}/conf/${PIPELINE}/pipeline.json ${NICDIR}/conf/pipeline.json
}

function run_gtest () {
    TEST_OBJECT=$1
    TEST_NAME=${PIPELINE}_${TEST_OBJECT}_test
    TEST_LOG=${TEST_NAME}_log.txt
    for cmdargs in "$@"
    do
        arg=$(echo $cmdargs | cut -f1 -d=)
        val=$(echo $cmdargs | cut -f2 -d=)
        case "$arg" in
            LOG) TEST_LOG=${val};;
            CFG) TEST_CFG=${val};;
            *)
        esac
    done
    echo "`date +%x_%H:%M:%S:%N` : Running ${TEST_NAME}  > ${TEST_LOG} "
    ${TEST_NAME} -c hal.json ${TEST_CFG} --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/${TEST_NAME}.xml" > ${TEST_LOG}
    #$GDB ${TEST_NAME} -c hal.json ${TEST_CFG} --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/${TEST_NAME}.xml"
    [[ $? -ne 0 ]] && echo "${TEST_NAME} failed!" && exit 1
    return 0
}

function run_sdk_gtest () {
    TEST_NAME=${PIPELINE}_sdk_test
    echo "`date +%x_%H:%M:%S:%N` : Running ${TEST_NAME}"
    ${SDKDIR}/tools/run_sdk_gtests.sh
    [[ $? -ne 0 ]] && echo "${TEST_NAME} failed!" && exit 1
    return 0
}

function run_valgrind_gtest () {
    TEST_NAME=${PIPELINE}_$1_test
    TEST_LOG=${NICDIR}/valgrind_${PIPELINE}_scale_test_1vpc.log
    # TODO: check if function arg parsing can be re-used
    for cmdargs in "$@"
    do
        arg=$(echo $cmdargs | cut -f1 -d=)
        val=$(echo $cmdargs | cut -f2 -d=)
        case "$arg" in
            CFG) TEST_CFG=${val};;
            *)
        esac
    done
    echo "`date +%x_%H:%M:%S:%N` : Running ${TEST_NAME} > ${TEST_LOG} "
    ${VAL_CMD} --track-origins=yes --leak-check=summary --show-leak-kinds=definite -v --log-file=${TEST_LOG} --suppressions=${NICDIR}/apollo/test/tools/valgrind_suppression.txt ${NICDIR}/build/x86_64/${PIPELINE}/bin/${TEST_NAME} -c hal.json ${TEST_CFG} --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/${TEST_NAME}.xml"
    #$GDB ${NICDIR}/build/x86_64/${PIPELINE}/bin/${PIPELINE}_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/scale_cfg_1vpc.json -f ${PIPELINE}

    # check valgrind log for leaks
    python ${NICDIR}/apollo/test/tools/parse_valgrind_log.py ${TEST_LOG}
    [[ $? -ne 0 ]] && echo "${TEST_NAME} failed!" && exit 1
    return 0
}

function clean_exit () {
    # to be invoked ONLY for successful run
    echo "Success"
    exit 0
}
