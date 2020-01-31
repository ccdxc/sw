#!/bin/sh

WS_TOP="/sw"
TOPDIR="/sw/nic"
BUILD_DIR=${TOPDIR}/build/x86_64/apulu/

export OPERD_REGIONS="./operd-regions.json"

pushd ${TOPDIR}

make PIPELINE=apulu operd-test-vpp-logger.bin operd.bin libvpp_operd_dec.so \
     liboperdvppconsumer.so operdvppconsumer.bin
RET=$?
if [ $RET -ne 0 ]
then
    echo "Build failed"
    exit $RET
fi

popd

for TESTCASE in './operd-daemon.json' './operd-daemon-bin.json'
do

    rm -rf /dev/shm/vpp*
    rm -rf ./vpp.log

    ${BUILD_DIR}/bin/operd-test-vpp-logger

    timeout -k 2s 2s ${BUILD_DIR}/bin/operd $TESTCASE \
            ./operd-decoders.json >& vpp.log

    cat vpp.log
    grep -c "Add, allow, ip, source: 1.1.1.1:10, destination: 2.2.2.2:20, proto: 6" vpp.log
    RED=$?
    if [ $RET -ne 0 ]
    then
        echo "Didn't find expected string in logs"
        exit $RET
    fi
done

