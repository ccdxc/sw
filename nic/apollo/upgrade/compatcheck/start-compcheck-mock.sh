#! /bin/bash

set -x
if [ $# != 6 ]; then
    echo "Invalid arguments, Usage : start_compcheck_mock.sh -r <running_topdir> -n <new_topdir> -p <pipeline>"
    exit -1;
fi

argc=$#
argv=($@)
for (( j=0; j<argc; j++ )); do
    if [ ${argv[j]} == '-p' ];then
        PIPELINE=${argv[j+1]}
    elif [ ${argv[j]} == '-r' ];then
        RCONFIG_PATH=${argv[j+1]}/conf # running config path, derived from running_topdir
    fi
done

TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
NIC_DIR=`dirname $ABS_TOOLS_DIR/../../../../`

#GDB='gdb --args'

export CONFIG_PATH=$NIC_DIR/conf/
export HAL_CONFIG_PATH=$NIC_DIR/conf/
export LOG_DIR=$NIC_DIR/
export PERSISTENT_LOG_DIR=$NIC_DIR/
export CAPRI_MOCK_MODE=1
export CAPRI_MOCK_MEMORY_MODE=1
export COVFILE=$NIC_DIR/coverage/sim_bullseye_hal.cov
export OPERD_REGIONS=$NIC_DIR/conf/operd-regions.json

BUILD_DIR=$NIC_DIR/build/x86_64/$PIPELINE
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NIC_DIR/third-party/metaswitch/output/x86_64/debug/:$BUILD_DIR/lib

echo "Starting upgrade compat checks: `date +%x_%H:%M:%S:%N`"

rm -f $NIC_DIR/conf/pipeline.json
rm -f $NIC_DIR/conf/operd-regions.json
ln -s $NIC_DIR/conf/$PIPELINE/pipeline.json $NIC_DIR/conf/pipeline.json
ln -s $NIC_DIR/conf/$PIPELINE/operd-regions.json $NIC_DIR/conf/operd-regions.json

function finish {
    echo "pdsupg compat check exit"
    $BUILD_DIR/bin/operdctl dump upgrade > upgrade_cc.log
    rm -f $NIC_DIR/conf/pipeline.json
    rm -f $NIC_DIR/conf/operd-regions.json
}
trap finish EXIT

CMD="$BUILD_DIR/bin/pdsupgcc -c hal.json -i $RCONFIG_PATH $* 2>&1"
$GDB $CMD
[[ $? -ne 0 ]] && echo "pdsupg compat check failed" && exit 1
exit 0

#valgrind --leak-check=full --show-leak-kinds=all --gen-suppressions=all --error-limit=no --verbose --log-file=valgrind-out.txt --track-origins=yes $CMD
