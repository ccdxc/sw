#!/bin/sh

WS_TOP="/sw"
TOPDIR="/sw/nic"
BUILD_DIR=${TOPDIR}/build/x86_64/iris/
DELPHI_BIN=${BUILD_DIR}/bin/delphi_hub
PENLOG_BIN=${BUILD_DIR}/bin/penlog
PENLOG_TEST_BIN=${BUILD_DIR}/bin/penlog_test

export PENLOG_LOCATION="."

trap 'kill $(jobs -p)' EXIT

pushd ${TOPDIR}

make delphi_hub.bin penlog.bin penlog_test.bin
RET=$?
if [ $RET -ne 0 ]
then
    echo "Build failed"
    exit $RET
fi

popd

${DELPHI_BIN} &
DELPHI_PID=$!

check_exist () {
    lines=("$@")
    for ln in "${lines[@]}"
    do tail -n 10 penlog_test.log | grep -c "$ln"
       RET=$?
       if [ $RET -ne 0 ]
       then
           echo "Didn't find $ln in the logs"
           exit $RET
       fi
    done
}

check_not_exist () {
    lines=("$@")
    for ln in "${lines[@]}"
    do tail -n 10 penlog_test.log | grep -c "$ln"
       RET=$?
       if [ $RET -eq 0 ]
       then
           echo "Found $ln in the logs"
           exit $RET
       fi
    done
}


# Test 1 Start
## Fire up. Logging level should be INFO
${PENLOG_TEST_BIN} &
PENLOG_TEST_PID=$!
sleep 3s
check_exist "Penlog test critical" "Penlog test error" "Penlog test warn" \
            "Penlog test info"
check_not_exist "Penlog test debug" "Penlog test trace"
## Change level to DEBUG
${PENLOG_BIN} penlog_test debug
sleep 3s
check_exist "Penlog test critical" "Penlog test error" "Penlog test warn" \
            "Penlog test info" "Penlog test debug" 
check_not_exist "Penlog test trace"
## Change level to DEBUG
${PENLOG_BIN} penlog_test trace
sleep 3s
check_exist "Penlog test critical" "Penlog test error" "Penlog test warn" \
            "Penlog test info" "Penlog test debug" "Penlog test trace"
## Kill PENLOG_TEST_PID
kill ${PENLOG_TEST_PID}
## Start it again. The level should be the same
${PENLOG_TEST_BIN} &
PENLOG_TEST_PID=$!
sleep 3
check_exist "Penlog test critical" "Penlog test error" "Penlog test warn" \
            "Penlog test info" "Penlog test debug" "Penlog test trace"
exit 0
