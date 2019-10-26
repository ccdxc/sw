#! /bin/bash -x

TOPDIR=`git rev-parse --show-toplevel`
NICDIR="$TOPDIR/nic"
# for mock runs, point obfl logging to /tmp
export PERSISTENT_LOG_DIR=/tmp/
echo $NICDIR

$NICDIR/apollo/tools/apulu/start-agent-mock.sh > agent.log 2>&1 &
# wait till agent opens up gRPC service port
sleep 10
$NICDIR/build/x86_64/apulu/bin/testapp -i $NICDIR/apollo/test/scale/apulu/scale_cfg.json -f apulu 2>&1 | tee testapp.log
linecount=`$NICDIR/build/x86_64/apulu/bin/pdsctl show vpc | grep "TENANT" | wc -l`
echo $linecount
[[ $linecount -eq 0 ]] && pkill agent && exit 1
pkill agent
exit 0
