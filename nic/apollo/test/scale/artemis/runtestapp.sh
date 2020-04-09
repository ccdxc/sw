#! /bin/bash -x
TOPDIR=`git rev-parse --show-toplevel`
NICDIR="$TOPDIR/nic"
export ASIC="${ASIC:-capri}"
export PDSPKG_TOPDIR=$NICDIR
DOLDIR=`readlink -f $NICDIR/../dol/`

echo $NICDIR

cleanup() {
    pkill agent
    # remove pdsctl gen files
    rm -f $NICDIR/out.sh
    # remove pipeline.json
    rm -f $NICDIR/conf/pipeline.json
}
trap cleanup EXIT

$NICDIR/apollo/tools/artemis/start-agent-mock.sh > agent.log 2>&1 &
sleep 10
$NICDIR/build/x86_64/artemis/${ASIC}/bin/testapp -i $NICDIR/apollo/test/scale/artemis/scale_cfg.json 2>&1 | tee testapp.log
linecount=`$NICDIR/build/x86_64/artemis/${ASIC}/bin/pdsctl show vpc | grep "TENANT" | wc -l`
if [[ $linecount -eq 0 ]]; then
    echo "testapp failure"
    exit 1
fi
echo "success"
exit 0
