#! /bin/bash -x
TOPDIR=`git rev-parse --show-toplevel`
NICDIR="$TOPDIR/nic"
DOLDIR=`readlink -f $NICDIR/../dol/`
echo $NICDIR
#export GDB='gdb --args'

cleanup() {
    pkill agent
    # remove pdsctl gen files
    rm -f $NICDIR/out.sh
    # remove pipeline.json
    rm -f $NICDIR/conf/pipeline.json
}
trap cleanup EXIT

$NICDIR/apollo/tools/start-agent-mock.sh > agent.log 2>&1 &
sleep 10
$NICDIR/build/x86_64/apollo/bin/testapp -i $NICDIR/apollo/test/scale/scale_cfg.json 2>&1 | tee testapp.log
linecount=`$NICDIR/build/x86_64/apollo/bin/pdsctl show vnic | grep "DOT1Q" | wc -l`
if [[ $linecount -eq 0 ]]; then
    echo "testapp failure"
    exit 1
fi
echo "success"
exit 0
