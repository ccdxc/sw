#! /bin/bash

set -e
export NICDIR=`pwd`
export HAL_LOG_DIR=${NICDIR}
export ZMQ_SOC_DIR=${NICDIR}
#export CAPRI_MOCK_MODE=1
#export CAPRI_MOCK_MEMORY_MODE=1
#export SKIP_VERIFY=1
export BUILD_DIR=${NICDIR}/build/x86_64/apollo/
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results
export HAL_CONFIG_PATH=${NICDIR}/conf
#export GDB='gdb --args'

export PATH=${PATH}:${BUILD_DIR}/bin
$GDB apollo_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/scale_cfg_sim.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_scale_test.xml"

# Valgrind with XML output
#valgrind --track-origins=yes --xml=yes --xml-file=out.xml apollo_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/scale_cfg.json

# Valgrind with text output
#valgrind --track-origins=yes apollo_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/scale_cfg.json
