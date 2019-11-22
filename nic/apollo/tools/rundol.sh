#! /bin/bash

TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NICDIR=`readlink -f $ABS_TOOLS_DIR/../../`
DOLDIR=`readlink -f $NICDIR/../dol/`
export PERSISTENT_LOG_DIR=$NICDIR
DRYRUN=0
START_VPP=0

argc=$#
argv=($@)
for (( j=0; j<argc; j++ )); do
    if [ ${argv[j]} == '--pipeline' ];then
        PIPELINE=${argv[j+1]}
    elif [ ${argv[j]} == '--feature' ];then
        FEATURE=${argv[j+1]}
    elif [[ ${argv[j]} =~ .*'--dry'.* ]];then
        DRYRUN=1
    fi
done

if [ $DRYRUN == 0 ] && [ $FEATURE == 'rfc' -o $PIPELINE == 'apulu' ]; then
    START_VPP=1
fi

set -x
echo $NICDIR

cleanup() {
    if [ $START_VPP == 1 ]; then
        sudo $NICDIR/vpp/tools/stop-vpp-sim.sh $NICDIR $PIPELINE
    fi
    pkill agent
    pkill cap_model
    rm -f $NICDIR/out.sh
    echo "===== Collecting logs ====="
    ${NICDIR}/apollo/test/tools/savelogs.sh
}

trap cleanup EXIT

NICMGR_FILE="$NICDIR/nicmgr.log"
if [ -f "$NICMGR_FILE" ]; then
    echo "Removing $NICMGR_FILE"
    command rm -f $NICMGR_FILE
fi

if [ $PIPELINE == 'apollo' ];then
    $NICDIR/apollo/tools/start-agent-sim.sh > agent.log 2>&1 &
else
    export AGENT_TEST_HOOKS_LIB='libdolagenthooks.so'
    $NICDIR/apollo/tools/$PIPELINE/start-agent-sim.sh > agent.log 2>&1 &
fi
$NICDIR/apollo/test/tools/$PIPELINE/start-$PIPELINE-model.sh &

export CONFIG_PATH=$NICDIR/conf

if [ $START_VPP == 1 ]; then
    echo "Starting VPP"
    sudo $NICDIR/vpp/tools/start-vpp-sim.sh $*
    if [[ $? != 0 ]]; then
        echo "Failed to bring up VPP"
        exit -1
    fi
fi

$DOLDIR/main.py $* 2>&1 | tee dol.log
status=${PIPESTATUS[0]}

exit $status
