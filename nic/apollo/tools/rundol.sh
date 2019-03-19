#! /bin/bash -x

TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NICDIR=`readlink -f $ABS_TOOLS_DIR/../../`
DOLDIR=`readlink -f $NICDIR/../dol/`

echo $NICDIR
$NICDIR/apollo/tools/start-agent-sim.sh > agent.log 2>&1 &
$NICDIR/apollo/test/tools/start-apollo-model.sh &

$DOLDIR/main.py $* 2>&1 | tee dol.log
status=${PIPESTATUS[0]}

pkill agent
pkill cap_model
exit $status
