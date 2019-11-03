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

if [[ $PIPELINE != 'apollo' && $PIPELINE != 'apulu' ]];then
    echo "Not starting VPP for pipeline - $PIPELINE"
    exit 0
fi

NICMGR_FILE="$NICDIR/nicmgr.log"
VPPLOG_FILE="$NICDIR/vpp.log"
command rm -f $VPPLOG_FILE
counter=$((600*TIMEOUT_SCALE))
#echo "$NICMGR_FILE"
#echo "$VPPLOG_FILE"
while [ $counter -gt 0 ]
do
    if [ -f "$NICMGR_FILE" ]; then
        if grep -q "cpu_mnic0: Skipping MNIC device" $NICMGR_FILE; then
            echo "NICMGR is up, bring-up VPP" >> $VPPLOG_FILE
            break
        fi
    fi
    echo "Waiting for nicmgr to be up, count - $counter" >> $VPPLOG_FILE
    sleep 1
    counter=$(( $counter - 1 ))
done

if [ $counter == 0 ]; then
    echo "NICMGR is down, aborting VPP bring-up." >> $VPPLOG_FILE
    exit 1
fi

VPP_PKG_DIR=$NICDIR/sdk/third-party/vpp-pkg/x86_64

source $NICDIR/vpp/tools/setup-vpp-prerequisites.sh $NICDIR $PIPELINE

echo "Starting VPP"
#start vpp
LD_LIBRARY_PATH=$NICDIR/build/x86_64/$PIPELINE/lib:$VPP_PKG_DIR/lib $VPP_PKG_DIR/bin/vpp -c $NICDIR/vpp/conf/vpp_1_worker_sim.conf &
[[ $? -ne 0 ]] && echo "Failed to start VPP!" && exit 1
#cat /proc/meminfo | grep -i huge
counter=$((60*TIMEOUT_SCALE))
while [ $counter -gt 0 ]
do
    LD_LIBRARY_PATH=$NICDIR/build/x86_64/$PIPELINE/lib:$VPP_PKG_DIR/lib $VPP_PKG_DIR/bin/vppctl trace add dpdk-input 50
    if [[ $? == 0 ]]; then
        exit 0
    fi
    sleep 1
    counter=$(( $counter - 1 ))
done
#cat /proc/cpuinfo
exit -1
