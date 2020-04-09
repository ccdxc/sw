#! /bin/bash

set -e
export ASIC="${ASIC:-capri}"
export NICDIR=`pwd`
export NON_PERSISTENT_LOGDIR=${NICDIR}
export ZMQ_SOC_DIR=${NICDIR}
export CONFIG_PATH=${NICDIR}/conf/
#export ASIC_MOCK_MODE=1
#export ASIC_MOCK_MEMORY_MODE=1
#export SKIP_VERIFY=1
export BUILD_DIR=${NICDIR}/build/x86_64/apulu/${ASIC}
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results
export CONFIG_PATH=${NICDIR}/conf
#export GDB='gdb --args'
#export TCID="1"
export PATH=${PATH}:${BUILD_DIR}/bin

cfgfile=apulu/scale_cfg_sim.json
if [[ "$1" ==  --cfg ]]; then
    cfgfile=$2
fi

function finish {
   echo "===== Collecting logs ====="
   ${NICDIR}/apollo/test/tools/savelogs.sh
   rm -f ${NICDIR}/conf/pipeline.json
}
trap finish EXIT

rm -f $NICDIR/conf/pipeline.json
ln -s $NICDIR/conf/apulu/pipeline.json $NICDIR/conf/pipeline.json
$GDB apulu_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/$cfgfile --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apulu_scale_test.xml"
rm -f $NICDIR/conf/pipeline.json

# Valgrind with XML output
#valgrind --track-origins=yes --xml=yes --xml-file=out.xml apulu_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/$cfgfile

# Valgrind with text output
#valgrind --track-origins=yes apulu_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/$cfgfile
