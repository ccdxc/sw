#! /bin/bash
export ASIC="${ASIC:-capri}"
export WS_TOP="/sw"
export TOPDIR="/sw/nic"
export BUILD_DIR=${TOPDIR}/build/x86_64/iris/${ASIC}
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results

#CMD_OPTS="COVFILE\=${COVFILE}"
set -e
# Upgrade Manager gtests
export PATH=${PATH}:${BUILD_DIR}/bin
${CMD_OPTS} upgrade_sdk_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/upgrade_sdk_test.xml"
${CMD_OPTS} upgrade_manager_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/upgrade_manager_test.xml"

#@# SDK tests
#@find build/x86_64/iris/bin/ -name *.runfiles_manifest* | sed 's/\.runfiles_manifest$$//1' | \
#while read i; \
#    do printf "\nRunning $$i\n" && $$i --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/${basename{$$i}}.xml"; \
#done \


