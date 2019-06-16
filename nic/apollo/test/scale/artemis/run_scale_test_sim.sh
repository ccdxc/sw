#! /bin/bash

set -e
export NICDIR=`pwd`
export HAL_LOG_DIR=${NICDIR}
export ZMQ_SOC_DIR=${NICDIR}
#export CAPRI_MOCK_MODE=1
#export CAPRI_MOCK_MEMORY_MODE=1
#export SKIP_VERIFY=1
export BUILD_DIR=${NICDIR}/build/x86_64/artemis/
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results
export HAL_CONFIG_PATH=${NICDIR}/conf
#export GDB='gdb --args'
export APOLLO_TEST_TEP_ENCAP=vxlan

cfgfile=artemis/scale_cfg.json
if [[ "$1" ==  --cfg ]]; then
    cfgfile=$2
fi

export PATH=${PATH}:${BUILD_DIR}/bin
$GDB apollo_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/$cfgfile -f artemis --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_scale_test.xml"
#$GDB apollo_scale_test -p p1 -c hal.json -i ${NICDIR}/apollo/test/scale/scale_cfg_p1.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_scale_test.xml"
#$GDB apollo_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/scale_cfg_v4_only.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_scale_test.xml"
#valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all --gen-suppressions=all --verbose --error-limit=no --log-file=valgrind-out.txt apollo_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/scale_cfg_v4_only.json
