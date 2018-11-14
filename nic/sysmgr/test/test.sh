#!/bin/sh


WS_TOP="/sw"
TOPDIR="/sw/nic"
BUILD_DIR=${TOPDIR}/build/x86_64/iris/

pushd ${TOPDIR}

make delphi_hub.bin sysmgr.bin sysmgr_scheduler_test.gtest \
     sysmgr_watchdog_test.gtest sysmgr_example.bin
RET=$?
if [ $RET -ne 0 ]
then
    echo "Build failed"
    exit $RET
fi

popd

#${BUILD_DIR}/bin/sysmgr_scheduler_test && ${BUILD_DIR}/bin/sysmgr_watchdog_test
#RET=$?
RET=0
if [ $RET -ne 0 ]
then
    echo "UT failed"
    exit $RET
fi

pushd /usr/src/github.com/pensando/sw/nic/sysmgr/goexample && go build && popd

runtest () {
    rm -rf *.log core.*
    timeout $1 ${BUILD_DIR}/bin/sysmgr $2 .
    grep -c "$3" sysmgr*.out.log
    RET=$?
    cat *.log
    if [ $RET -ne 0 ]
    then
	echo "Didn't file $3 in the logs"
	echo "test.json failed"
	exit $RET
    fi
}

runtest 10s test.json "example2 -> started"

runtest 10s test-exit-code.json "example2 -> Exited normally with code: 12"
