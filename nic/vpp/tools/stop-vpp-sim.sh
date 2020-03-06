#!/bin/sh

NICDIR=$1
PIPELINE=$2

$NICDIR/vpp/tools/vppctl-sim.sh show hardware >> $NICDIR/vpp.log
$NICDIR/vpp/tools/vppctl-sim.sh show interface >> $NICDIR/vpp.log
$NICDIR/vpp/tools/vppctl-sim.sh show error >> $NICDIR/vpp.log
$NICDIR/vpp/tools/vppctl-sim.sh show trace >> $NICDIR/vpp.log
$NICDIR/vpp/tools/vppctl-sim.sh show logging >> $NICDIR/vpp.log
pstack `pgrep vpp_main` &> $NICDIR/vpp_bt.log
pkill -9 vpp_main
rm -f $NICDIR/conf/vpp_startup.conf
