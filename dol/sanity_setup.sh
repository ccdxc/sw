#! /bin/bash

set -e

TOPDIR=`git rev-parse --show-toplevel`
export NICDIR="$TOPDIR/nic/"
export DOLDIR="$TOPDIR/dol/"
export CONFIG_PATH=$NICDIR/conf/

function finish () {
    # auto invoked on any exit
    rm -f ${NICDIR}/out.sh
    echo "===== Collecting logs ====="
    ${NICDIR}/apollo/test/tools/savelogs.sh
}
trap finish EXIT

function setup () {
    export TEST_PIPELINE=$1
    echo "===== Setting ${TEST_PIPELINE} pipeline ====="
    cd ${NICDIR}
    rm -f ${NICDIR}/*log* ${NICDIR}/core* ${NICDIR}/out.sh
}

function build () {
    # build only the minimum targets required for dryrun
    make -C /sw/nic PIPELINE=${TEST_PIPELINE} libhost_mem.so pds.proto
}

function ErrorCheckNExit() {
    if [ "$1" = "0" ];then
        return
    fi

    echo "ERROR: $2 FAILED"
    exit 1
}

function DryRunSanity()
{
    TEST_TOPO=$1
    TEST_FEATURE=$2
    TEST_NAME=${TEST_PIPELINE}_${TEST_FEATURE}_${TEST_TOPO}_test
    TEST_LOG=${NICDIR}/${TEST_NAME}.log
    echo "`date +%x_%H:%M:%S:%N` : Running ${TEST_NAME}  > ${TEST_LOG} "
    ${DOLDIR}/main.py --dryrun --topo ${TEST_TOPO} --feature ${TEST_FEATURE} --pipeline ${TEST_PIPELINE} | tee ${TEST_LOG}
    ErrorCheckNExit ${PIPESTATUS[0]} "${TEST_NAME}"
}

function clean_exit () {
    # to be invoked ONLY for successful run
    echo "Success"
    exit 0
}