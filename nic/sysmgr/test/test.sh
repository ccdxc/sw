#!/bin/sh

WS_TOP="/sw"
TOPDIR="/sw/nic"
TESTDIR=$(pwd)
export ARCH="${ARCH:-x86_64}"
export PIPELINE="${PIPELINE:-iris}"
export ASIC="${ASIC:-capri}"
BUILD_DIR=${TOPDIR}/build/x86_64/iris/${ASIC}

export OPERD_REGIONS="${TESTDIR}/operd-regions.json"
export PENLOG_LOCATION="."
export NO_WATCHDOG=1
export SYSMGR_CORES="0x1"

pushd ${TOPDIR}

make delphi_hub.bin sysmgr.bin sysmgr_example.bin operdctl.bin
RET=$?
if [ $RET -ne 0 ]
then
    echo "Build failed"
    exit $RET
fi

popd

# ${BUILD_DIR}/bin/sysmgr_scheduler_test && ${BUILD_DIR}/bin/sysmgr_watchdog_test
# RET=$?
# if [ $RET -ne 0 ]
# then
#     echo "UT failed"
#     exit $RET
# fi

pushd /usr/src/github.com/pensando/sw/nic/sysmgr/goexample && go build && popd

echo "sysmgr test.sh arch=${ARCH}, pipeline=${PIPELINE}, asic=${ASIC}"
if [ ${ASIC} != "capri" ]; then
    echo "Please fix test.json etc.. to run this test for ${ASIC}. Ignored for now..."
    exit 0
fi

cd /sw/nic/build/${ARCH}/${PIPELINE}
for d in bin lib out pgm_bin gen gtest_results
do
    if [ -e ${ASIC}/$d ]; then
        if [ -e $d ]; then
            echo "link $d [`readlink $d`] already exist"
        else
            echo "ln -s ${ASIC}/$d $d"
            ln -s ${ASIC}/$d $d
        fi
    else
        echo "ignore ${ASIC}/$d"
    fi
done

cd ${TESTDIR}

runtest () {
    tm=$1
    shift
    json=$1
    shift
    lines=("$@")
    rm -rf *.log core.* /tmp/delphi* *.out.log* *.err.log* /dev/shm/*
    echo Running $json test
    timeout -k $tm $tm ${BUILD_DIR}/bin/sysmgr ${TESTDIR}/$json .
    ${BUILD_DIR}/bin/operdctl dump sysmgr > sysmgr.log
    cat *.log
    for ln in "${lines[@]}"
    do grep -c "$ln" *.log
       RET=$?
       if [ $RET -ne 0 ]
       then
           echo "Didn't find $ln in the logs"
           echo "$json failed"
           exit $RET
       fi
    done
}

#runtest 10s test-respawn.json "Service example1 started"

runtest 10s test.json "Service example2 started" \
        "Restricting sysmgr to 0x1 cores"

runtest 10s test-exit-code.json "Service example2 Exited normally with code: 12" \
        "ProcessStatus example2, .*, 4, Exited normally with code: 12" \
        "Service example3 exited normally"

runtest 120s test-critical-watchdog.json "Service example2 timed out" \
        "System in fault mode" "Simulation Watchdog Expired"

runtest 20s test-oneshot.json "Service script is oneshot, not setting fault"
