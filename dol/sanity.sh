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

./ut_run.sh; err=$?; cat ut_run.log;
ErrorCheckNExit $err "utrun"

# Build the minimum targets required for dryrun.
cd ../nic/utils/host_mem && make && cd -
cd ../nic/proto/ && make && cd -

#./main.py --dryrun --topo eth --module eth
#ErrorCheckNExit $? "eth topo"

./main.py --dryrun --topo vxlan --module vxlan
ErrorCheckNExit $? "vxlan topo"

./main.py --dryrun --topo proxy --module tcp_proxy
ErrorCheckNExit $? "tcpproxy topo."

./main.py --dryrun --topo up2up --feature l2uc,l3uc
ErrorCheckNExit $? "up2up topo."

./main.py --module telemetry --topo telemetry --dryrun
ErrorCheckNExit $? "telemetry topo."

exit 0
