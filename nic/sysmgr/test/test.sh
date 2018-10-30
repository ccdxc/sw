#!/bin/sh


WS_TOP="/sw"
TOPDIR="/sw/nic"
BUILD_DIR=${TOPDIR}/build/x86_64/iris/

pushd ${TOPDIR}

make delphi_hub.bin sysmgr_test.bin sysmgr_scheduler_test.gtest sysmgr_watchdog_test.gtest \
	sysmgr_example.bin  sysmgr_test_complete.bin
RET=$?
if [ $RET -ne 0 ]
then
    echo "Build failed"
    exit $RET
fi

popd

${BUILD_DIR}/bin/sysmgr_scheduler_test && ${BUILD_DIR}/bin/sysmgr_watchdog_test
RET_1=$?

pushd /usr/src/github.com/pensando/sw/nic/sysmgr/goexample && go build && popd

rm -rf *.log core.*
timeout 60s ${BUILD_DIR}/bin/sysmgr_test
RET_2=$?

cat *.log

echo "gtest result = $RET_1"
echo "sysmgr_test result = $RET_2"
exit `expr "$RET_1" + "$RET_2"`
