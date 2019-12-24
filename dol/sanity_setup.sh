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
    mkdir -p ${NICDIR}/conf/gen/
    rm -f ${NICDIR}/*log* ${NICDIR}/core* ${NICDIR}/out.sh
}

function build () {
    # build only the minimum targets required for dryrun
    make -C /sw/nic PIPELINE=${TEST_PIPELINE} pds.proto
}

function ErrorCheckNExit () {
    if [ "$1" = "0" ];then
        return
    fi

    echo "ERROR: $2 FAILED"
    exit 1
}

function SetDryRunOptionalArgs () {
    TEST_SUB_FEATURE=""
    for cmdargs in "$@"
    do
        arg=$(echo $cmdargs | cut -f1 -d=)
        val=$(echo $cmdargs | cut -f2 -d=)
        case "$arg" in
            SUB) TEST_SUB_FEATURE=" --sub ${val}";;
            *)
        esac
    done
}

function DryRunSanity () {
    TEST_TOPO=$1
    TEST_FEATURE=$2
    TEST_NAME=${TEST_PIPELINE}_${TEST_FEATURE}_${TEST_TOPO}_test
    TEST_LOG=${NICDIR}/${TEST_NAME}.log
    SetDryRunOptionalArgs $@
    echo "`date +%x_%H:%M:%S:%N` : Running ${TEST_NAME}  > ${TEST_LOG} "
    ${DOLDIR}/main.py --dryrun --topo ${TEST_TOPO} --feature ${TEST_FEATURE} --pipeline ${TEST_PIPELINE} ${TEST_SUB_FEATURE} | tee ${TEST_LOG}
    ErrorCheckNExit ${PIPESTATUS[0]} "${TEST_NAME}"
}

function clean_exit () {
    # to be invoked ONLY for successful run
    echo "Success"
    exit 0
}