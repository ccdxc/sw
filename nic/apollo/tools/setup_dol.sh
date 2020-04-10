#! /bin/bash

DRYRUN=0
START_VPP=0
NO_STOP=0
START_DHCP_SERVER=0

# set file size limit to 30GB so that model logs will not exceed that.
ulimit -f $((30*1024*1024))

CMDARGS=""
argc=$#
argv=($@)
for (( j=0; j<argc; j++ )); do
    [ "${argv[j]}" != '--nostop' ] && CMDARGS+="${argv[j]} "
    if [ ${argv[j]} == '--pipeline' ];then
        PIPELINE=${argv[j+1]}
    elif [ ${argv[j]} == '--feature' ];then
        FEATURE=${argv[j+1]}
    elif [[ ${argv[j]} =~ .*'--dry'.* ]];then
        DRYRUN=1
    elif [[ ${argv[j]} == '--nostop' ]];then
        NO_STOP=1
    fi
done

if [ $DRYRUN == 0 ] && [ $FEATURE == 'rfc' -o $PIPELINE == 'apulu' ]; then
    START_VPP=1
fi

if [ $DRYRUN == 0 ] && [ $PIPELINE == 'apulu' ]; then
    START_DHCP_SERVER=1
fi

CUR_DIR=$( readlink -f $( dirname $0 ) )
if [[ "$BASH_SOURCE" != "$0" ]]; then
    CUR_DIR=$( readlink -f $( dirname $BASH_SOURCE ) )
fi
source $CUR_DIR/setup_env_sim.sh $PIPELINE

set -x

function stop_model() {
    pkill cap_model
}

function stop_process () {
    echo "===== Nuking processes ====="
    if [ $START_VPP == 1 ]; then
        sudo $PDSPKG_TOPDIR/vpp/tools/stop-vpp-sim.sh $PDSPKG_TOPDIR $PIPELINE
    fi
    #dump backtrace of agent process to file, useful for debugging if process hangs
    pstack `pgrep pdsagent` &> $PDSPKG_TOPDIR/pdsagent_bt.log
    pkill agent
    if [ $START_DHCP_SERVER == 1]; then
        pkill dhcpd
    fi
}

function start_vpp () {
    if [ $START_VPP == 1 ]; then
        echo "Starting VPP"
        sudo $PDSPKG_TOPDIR/vpp/tools/start-vpp-sim.sh ${CMDARGS}
        if [[ $? != 0 ]]; then
            echo "Failed to bring up VPP"
            exit 1
        fi
    fi
}

function start_dhcp_server() {
    if [[ $START_DHCP_SERVER != 1 ]]; then
        return
    fi
    echo "Starting DHCP server"
    sudo $PDSPKG_TOPDIR/apollo/tools/$PIPELINE/start-dhcpd-sim.sh -p $PIPELINE
    if [[ $? != 0 ]]; then
        echo "Failed to start dhcpd!"
        exit 1
    fi
}

function start_model () {
    $PDSPKG_TOPDIR/apollo/test/tools/$PIPELINE/start-$PIPELINE-model.sh &
}

function start_process () {
    $PDSPKG_TOPDIR/apollo/tools/$PIPELINE/start-agent-sim.sh > agent.log 2>&1 &
    start_dhcp_server
    start_vpp
}

function remove_db () {
    echo "===== Cleaning stale db ====="
    rm -f $PDSPKG_TOPDIR/pdsagent.db
    rm -f $PDSPKG_TOPDIR/pdsagent.db-lock
    rm -f /tmp/*.db
}

function remove_stale_files () {
    echo "===== Cleaning stale files ====="
    rm -f $PDSPKG_TOPDIR/out.sh
    rm -f $PDSPKG_TOPDIR/conf/pipeline.json
    rm -f $PDSPKG_TOPDIR/conf/gen/dol_agentcfg.json
    rm -f $PDSPKG_TOPDIR/conf/gen/device_info.txt
    rm -f /tmp/pen_* /dev/shm/pds_* /dev/shm/ipc_* /dev/shm/metrics_*
    rm -rf /sysconfig/config0
    remove_db
}

function remove_logs () {
    # NOT to be used post run
    echo "===== Cleaning log & core files ====="
    rm -f ${PDSPKG_TOPDIR}/*log* ${PDSPKG_TOPDIR}/core*
}

function collect_logs () {
    echo "===== Collecting logs ====="
    ${PDSPKG_TOPDIR}/apollo/test/tools/savelogs.sh
}

function finish () {
    if [ $NO_STOP == 1 ]; then
         return
    fi
    echo "===== Finishing ====="
    if [ $DRYRUN == 0 ]; then
        stop_process
        stop_model
    fi
    collect_logs
    ${PDSPKG_TOPDIR}/tools/print-cores.sh
    remove_stale_files
}
trap finish EXIT

function setup () {
    # Cleanup of previous run if required
    stop_process
    stop_model

    # remove stale files from older runs
    remove_stale_files
    remove_logs
}
setup

if [ $PIPELINE == 'artemis' ];then
    export AGENT_TEST_HOOKS_LIB='libdolagenthooks.so'
fi

if [ $DRYRUN == 0 ]; then
    start_model
    start_process
fi

# TODO Remove this once agent code is fixed
# Create dummy device.conf - agent is trying to update it when device object is updated.
# Without this, pdsagent crashes since config file is not found.
mkdir -p "/sysconfig/config0/"
touch "/sysconfig/config0/device.conf"

