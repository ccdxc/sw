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

function stop_process () {
    echo "===== Nuking processes ====="
    if [ $START_VPP == 1 ]; then
        sudo $NICDIR/vpp/tools/stop-vpp-sim.sh $NICDIR $PIPELINE
    fi
    #dump backtrace of agent process to file, useful for debugging if process hangs
    pstack `pgrep pdsagent` &> $NICDIR/pdsagent_bt.log
    pkill agent
    pkill cap_model
}

function remove_stale_files () {
    echo "===== Cleaning stale files ====="
    rm -f $NICDIR/out.sh
    rm -f $NICDIR/conf/pipeline.json
    rm -f $NICDIR/conf/gen/dol_agentcfg.json
}

function remove_logs () {
    # NOT to be used post run
    echo "===== Cleaning log & core files ====="
    rm -f ${NICDIR}/*log* ${NICDIR}/core*
}

function collect_logs () {
    echo "===== Collecting logs ====="
    ${NICDIR}/apollo/test/tools/savelogs.sh
}

function finish () {
    stop_process
    collect_logs
    remove_stale_files
}
trap finish EXIT

function setup () {
    # remove stale files from older runs
    remove_stale_files
    remove_logs
}
setup

if [ $PIPELINE == 'artemis' ];then
    export AGENT_TEST_HOOKS_LIB='libdolagenthooks.so'
fi

$NICDIR/apollo/tools/$PIPELINE/start-agent-sim.sh > agent.log 2>&1 &
$NICDIR/apollo/test/tools/$PIPELINE/start-$PIPELINE-model.sh &

export CONFIG_PATH=$NICDIR/conf

if [ $START_VPP == 1 ]; then
    echo "Starting VPP"
    sudo $NICDIR/vpp/tools/start-vpp-sim.sh $*
    if [[ $? != 0 ]]; then
        echo "Failed to bring up VPP"
        exit 1
    fi
fi

$DOLDIR/main.py $* 2>&1 | tee dol.log
status=${PIPESTATUS[0]}

# end of script
exit $status
