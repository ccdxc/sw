#!/bin/sh

TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NICDIR=`readlink -f $ABS_TOOLS_DIR/../../`
export COVFILE=${NICDIR}/coverage/sim_bullseye_hal.cov
TIMEOUT_SCALE=1

argc=$#
argv=($@)
for (( j=0; j<argc; j++ )); do
    if [ ${argv[j]} == '--pipeline' ];then
        PIPELINE=${argv[j+1]}
    fi
    if [ ${argv[j]} == '--coverage-run' ];then
        export COVFILE=${NICDIR}/coverage/sim_bullseye_hal.cov
        #Coverage makes vpp slower
        TIMEOUT_SCALE=2
    fi
done

#set -x
#echo $NICDIR

if [[ $PIPELINE != 'apulu' ]];then
    echo "Not starting VPP for pipeline - $PIPELINE"
    exit 0
fi

VPPLOG_FILE="$NICDIR/vpp.log"
command rm -f $VPPLOG_FILE
counter=$((600*TIMEOUT_SCALE))

VPP_PKG_DIR=$NICDIR/sdk/third-party/vpp-pkg/x86_64

#Set mock mode
export CAPRI_MOCK_MODE=1
export CAPRI_MOCK_MEMORY_MODE=1
source $NICDIR/vpp/tools/setup-vpp-prerequisites.sh $NICDIR $PIPELINE

echo "Starting VPP"
#start vpp
LD_LIBRARY_PATH=$NICDIR/build/x86_64/$PIPELINE/lib:$VPP_PKG_DIR/lib $VPP_PKG_DIR/bin/vpp -c $NICDIR/vpp/conf/vpp_1_worker_mock.conf &
[[ $? -ne 0 ]] && echo "Failed to start VPP!" && exit 1
counter=$((60*TIMEOUT_SCALE))
while [ $counter -gt 0 ]
do
    $NICDIR/vpp/tools/vppctl-sim.sh "show version"
    if [[ $? == 0 ]]; then
        exit 0
    fi
    sleep 1
    counter=$(( $counter - 1 ))
done
exit -1
