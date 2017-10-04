#! /bin/bash
export DIR_DOL=`dirname $0`
cd $DIR_DOL

function ErrorCheckNExit() {
    if [ "$1" = "0" ];then
        return
    fi

    echo "ERROR: $2 FAILED"
    exit 1
}

./ut/ut_run.sh; err=$?; cat ut_run.log;
ErrorCheckNExit $err "utrun"

# Build the minimum targets required for dryrun.
#cd ../nic/utils/host_mem && make && cd -
#cd ../nic/proto/ && make && cd -
bazel build //nic/utils/host_mem:host_mem
bazel build //nic/proto:all
bazel build //nic/proto/hal:all
bazel build //nic/proto/agents:all

function DryRunSanity()
{
    topo=$1
    feature=$2
    ./main.py --dryrun --topo $topo --feature $feature
    ErrorCheckNExit $? "$topo topo"
}

DryRunSanity eth eth
DryRunSanity vxlan vxlan
DryRunSanity up2up networking
DryRunSanity telemetry telemetry
DryRunSanity fte fte
DryRunSanity admin admin
DryRunSanity proxy proxy

exit 0
