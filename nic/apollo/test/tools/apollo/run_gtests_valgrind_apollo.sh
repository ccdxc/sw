#! /bin/bash

set -e
export NICDIR=`pwd`
export NON_PERSISTENT_LOGDIR=${NICDIR}
export ZMQ_SOC_DIR=${NICDIR}
export SKIP_VERIFY=1
export BUILD_DIR=${NICDIR}/build/x86_64/apollo/
export GEN_TEST_RESULTS_DIR=${NICDIR}
export HAL_CONFIG_PATH=${NICDIR}/conf
export CAPRI_MOCK_MODE=1
#export GDB='gdb --args'
export VAL_CMD=valgrind
export VALGRIND_DIR=${BUILD_DIR}/valgrind
export VAL_TR=${VALGRIND_DIR}/test_results
export VAL_INFO=${VALGRIND_DIR}/info
export LOG_FILE=${GEN_TEST_RESULTS_DIR}/valgrind_apollo_scale_test_1vpc.log

if [ ! -d ${GEN_TEST_RESULTS_DIR} ]
then
    mkdir -p ${GEN_TEST_RESULTS_DIR}
fi

rm -f ${NICDIR}/conf/pipeline.json
ln -s ${NICDIR}/conf/apollo/pipeline.json ${NICDIR}/conf/pipeline.json

${VAL_CMD} --track-origins=yes --leak-check=summary --show-leak-kinds=definite -v --log-file=${LOG_FILE} --suppressions=${NICDIR}/apollo/test/tools/valgrind_suppression.txt ${NICDIR}/build/x86_64/apollo/bin/apollo_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/scale_cfg_1vpc.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_scale_test.xml"
#$GDB ${NICDIR}/build/x86_64/apollo/bin/apollo_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/scale_cfg_1vpc.json -f apollo

python ${NICDIR}/apollo/test/tools/parse_valgrind_log.py ${LOG_FILE}
function finish {
   ${NICDIR}/apollo/test/tools/savelogs.sh
   rm -f ${NICDIR}/conf/pipeline.json
}
trap finish EXIT
