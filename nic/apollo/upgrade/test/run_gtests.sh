#! /bin/bash

set -e

export PIPELINE=apulu
CUR_DIR=$( readlink -f $( dirname $0 ))

source  ${CUR_DIR}/../../tools/setup_env_mock.sh $PIPELINE

function finish () {
    # auto invoked on any exit
    echo "===== Collecting logs ====="
    ${PDSPKG_TOPDIR}/apollo/test/tools/savelogs.sh
    rm -f ${CONFIG_PATH}/pipeline.json
}
trap finish EXIT

function setup () {
    rm -f ${PDSPKG_TOPDIR}/*log* ${PDSPKG_TOPDIR}/core*
    rm -f ${CONFIG_PATH}/pipeline.json
    ln -s ${CONFIG_PATH}/${PIPELINE}/pipeline.json ${CONFIG_PATH}/pipeline.json
}

setup

$GDB upg_func_test -c hal.json
[[ $? -ne 0 ]] && echo "upg_func_test failed!" && exit 1

exit 0
