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

${BUILD_DIR}/bin/sysmgr_scheduler_test && ${BUILD_DIR}/bin/sysmgr_watchdog_test
RET=$?
if [ $RET -ne 0 ]
then
    echo "UT failed"
    exit $RET
fi

pushd /usr/src/github.com/pensando/sw/nic/sysmgr/goexample && go build && popd

runtest () {
    tm=$1
    shift
    json=$1
    shift
    lines=("$@")
    rm -rf *.log core.*
    timeout $tm ${BUILD_DIR}/bin/sysmgr $json .
    cat *.log
    for ln in "${lines[@]}"
    do grep -c "$ln" sysmgr*.out.log
       RET=$?
       if [ $RET -ne 0 ]
       then
	   echo "Didn't find $ln in the logs"
	   echo "$json failed"
	   exit $RET
       fi
    done
}

runtest 10s test.json "example2 -> started"

runtest 10s test-exit-code.json "example2 -> Exited normally with code: 12"

runtest 60s test-critical-watchdog.json "Expired watchdog process: example2" \
	"Rebooting"
