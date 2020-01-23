#!/bin/sh

WS_TOP="/sw"
TOPDIR="/sw/nic"
BUILD_DIR=${TOPDIR}/build/x86_64/iris/

export OPERD_REGIONS="./operd-regions.json"

pushd ${TOPDIR}

make operd-daemon.bin libvpp_operd_dec.so libvpp_operd_dec.so
RET=$?
if [ $RET -ne 0 ]
then
    echo "Build failed"
    exit $RET
fi

popd

rm -rf /dev/shm/vpp*
rm -rf ./vpp.log

${BUILD_DIR}/bin/operd-test-vpp-logger

timeout -k 2s 2s ${BUILD_DIR}/bin/operd-daemon ./operd-daemon.json \
        ./operd-decoders.json

grep -c "IPv4 0x1010101 02020202 10 20" vpp.log
RED=$?
if [ $RET -ne 0 ]
then
    echo "Didn't find 0x1010101 in logs"
    exit $RET
fi

           
