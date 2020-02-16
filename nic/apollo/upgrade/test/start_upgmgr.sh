#! /bin/bash

CUR_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $CUR_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR/../../../../`

#GDB='gdb --args'

export OPERD_REGIONS=$NIC_DIR/conf/operd-regions.json
export PIPELINE=apulu

BUILD_DIR=$NIC_DIR/build/x86_64/$PIPELINE

echo "Starting pdsupgmgr: `date +%x_%H:%M:%S:%N`"

rm -f $NIC_DIR/conf/operd-regions.json
ln -s $NIC_DIR/conf/$PIPELINE/operd-regions.json $NIC_DIR/conf/operd-regions.json

function finish {
    echo "pdsupgmgr exit"
    $BUILD_DIR/bin/operdctl dump upgrade > upgrade.log
    rm -f $NIC_DIR/conf/operd-regions.json
}
trap finish EXIT

CMD="$BUILD_DIR/bin/pdsupgmgr $* 2>&1"
$GDB $CMD
[[ $? -ne 0 ]] && echo "pdsupgmgr bringup failed" && exit 1
exit 0

#valgrind --leak-check=full --show-leak-kinds=all --gen-suppressions=all --error-limit=no --verbose --log-file=valgrind-out.txt --track-origins=yes $CMD
