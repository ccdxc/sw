#! /bin/bash -x
TOPDIR=`git rev-parse --show-toplevel`
NICDIR="$TOPDIR/nic"
DOLDIR=`readlink -f $NICDIR/../dol/`

echo $NICDIR
$NICDIR/apollo/tools/start-agent-mock.sh > agent.log 2>&1 &
sleep 10
$NICDIR/build/x86_64/apollo/bin/testapp -i $NICDIR/apollo/test/scale/scale_cfg.json -f apollo 2>&1 | tee testapp.log
linecount=`$NICDIR/build/x86_64/apollo/bin/pdsctl show vnic | grep "DOT1Q" | wc -l`
[[ $linecount -eq 0 ]] && pkill agent && exit 1 

pkill agent
exit 0
