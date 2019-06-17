#! /bin/bash

TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NICDIR=`readlink -f $ABS_TOOLS_DIR/../../`
DOLDIR=`readlink -f $NICDIR/../dol/`

argc=$#
argv=($@)
for (( j=0; j<argc; j++ )); do
    if [ ${argv[j]} == '--pipeline' ];then
        PIPELINE=${argv[j+1]}
    fi
done

set -x
echo $NICDIR

if [ $PIPELINE == 'apollo' ];then
    $NICDIR/apollo/tools/start-agent-sim.sh > agent.log 2>&1 &
else
    export AGENT_TEST_HOOKS_LIB='libdolagenthooks.so'
    $NICDIR/apollo/tools/$PIPELINE/start-agent-sim.sh > agent.log 2>&1 &
fi
$NICDIR/apollo/test/tools/start-$PIPELINE-model.sh &

export CONFIG_PATH=$NICDIR/conf

$DOLDIR/main.py $* 2>&1 | tee dol.log
status=${PIPESTATUS[0]}

pkill agent
pkill cap_model
exit $status
