#! /bin/bash -x

TOPDIR=`git rev-parse --show-toplevel`
NICDIR="$TOPDIR/nic"
# for mock runs, point obfl logging to /tmp
export PERSISTENT_LOG_DIR=/tmp/
echo $NICDIR

cleanup() {
    pkill agent
    # remove pdsctl gen files
    rm -f $NICDIR/out.sh
    # remove pipeline.json
    rm -f $NICDIR/conf/pipeline.json
    # stop vpp
    pkill -9 vpp
}
trap cleanup EXIT

$NICDIR/apollo/tools/apulu/start-agent-mock.sh > agent.log 2>&1 &
# wait till agent opens up gRPC service port
sleep 10

echo "Starting VPP"
sudo $NICDIR/vpp/tools/start-vpp-mock.sh --pipeline apulu
if [[ $? != 0 ]]; then
    echo "Failed to bring up VPP"
    exit -1
fi

$NICDIR/build/x86_64/apulu/bin/testapp -i $NICDIR/apollo/test/scale/apulu/scale_cfg.json
linecount=`$NICDIR/build/x86_64/apulu/bin/pdsctl show vpc | grep "TENANT" | wc -l`
if [[ $linecount -eq 0 ]]; then
    echo "testapp failure"
    exit 1
fi
echo "success"
exit 0
