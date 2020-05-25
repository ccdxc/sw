#! /bin/bash

DRYRUN=0
NO_STOP=0
# max wait time for system to come up
SETUP_WAIT_TIME=600
# list of processes
SIM_PROCESSES=("vpp_main" "operd" "pdsagent" "sysmgr" "dhcpd")

# set file size limit to 50GB so that model logs will not exceed that.
ulimit -f $((50*1024*1024))

CMDARGS=""
argc=$#
argv=($@)
for (( j=0; j<argc; j++ )); do
    [ "${argv[j]}" != '--nostop' ] && CMDARGS+="${argv[j]} "
    if [ ${argv[j]} == '--pipeline' ];then
        PIPELINE=${argv[j+1]}
    elif [[ ${argv[j]} =~ .*'--dry'.* ]];then
        DRYRUN=1
    elif [[ ${argv[j]} == '--nostop' ]];then
        NO_STOP=1
    fi
done

CUR_DIR=$( readlink -f $( dirname $0 ) )
if [[ "$BASH_SOURCE" != "$0" ]]; then
    CUR_DIR=$( readlink -f $( dirname $BASH_SOURCE ) )
fi
source $CUR_DIR/setup_env_sim.sh $PIPELINE

function stop_model() {
    pkill cap_model
}

function stop_processes () {
    echo "======> Settting Phasers to kill"
    $PDSPKG_TOPDIR/vpp/tools/stop-vpp-sim.sh
    for process in ${SIM_PROCESSES[@]}; do
        #dump backtrace of process to file, useful for debugging if process hangs
        pstack `pgrep -x ${process}` &> $PDSPKG_TOPDIR/${process}_bt.log
        echo " == Nuking ${process}"
        pkill -9 ${process}
    done
    echo "======> Settting Phasers to stun"
}

function start_dhcp_server() {
    sudo $PDSPKG_TOPDIR/apollo/tools/$PIPELINE/start-dhcpd-sim.sh -p $PIPELINE > dhcpd.log 2>&1
    if [[ $? != 0 ]]; then
        echo "ERR: Failed to start dhcpd!"
        exit 1
    fi
}

function start_sysmgr () {
    PENLOG_LOCATION=/var/log/pensando/ sysmgr $PDSPKG_TOPDIR/sysmgr/src/$PIPELINE/pipeline-dol.json &
}

function start_model () {
    $PDSPKG_TOPDIR/apollo/test/tools/$PIPELINE/start-model.sh &
}

function start_processes () {
    echo "======> Laying in the course"
    start_sysmgr
    start_dhcp_server
}

function remove_db () {
    rm -f $PDSPKG_TOPDIR/pdsagent.db
    rm -f $PDSPKG_TOPDIR/pdsagent.db-lock
    rm -f /tmp/*.db
}

function remove_ipc_files () {
    rm -f /tmp/pen_*
}

function remove_shm_files () {
    rm -f /dev/shm/pds_* /dev/shm/ipc_* /dev/shm/metrics_* /dev/shm/alerts
    rm -f /dev/shm/nicmgr_shm /dev/shm/sysmgr /dev/shm/vpp
}

function remove_stale_files () {
    echo "======> Cleaning debris"
    rm -f $PDSPKG_TOPDIR/out.sh
    rm -f $PDSPKG_TOPDIR/ipstrc.bak
    rm -f $PDSPKG_TOPDIR/conf/pipeline.json
    rm -f $PDSPKG_TOPDIR/conf/gen/dol_agentcfg.json
    rm -f $PDSPKG_TOPDIR/conf/gen/device_info.txt
    rm -f /var/run/pds_svc_server_sock
    remove_db
    remove_ipc_files
    remove_shm_files
}

function remove_logs () {
    # NOT to be used post run
    echo "======> Incinerating logs from unknown stardate"
    sudo rm -f ${PDSPKG_TOPDIR}/*log* ${PDSPKG_TOPDIR}/core*
    sudo rm -rf /var/log/pensando/ /obfl/ /data/ /sysconfig/
}

function collect_logs () {
    echo "======> Recording captain's logs, stardate `date +%x_%H:%M:%S:%N`"
    # TODO: @param, move to techsupport instead
    ${PDSPKG_TOPDIR}/apollo/test/tools/savelogs.sh
}

function finish () {
    if [ $NO_STOP == 1 ]; then
         return
    fi
    echo "======> Starting shutdown sequence"
    if [ $DRYRUN == 0 ]; then
        stop_processes
        stop_model
        # unmount hugetlbfs
        umount /dev/hugepages || { echo "Failed to unmount hugetlbfs"; }
    fi
    collect_logs
    ${PDSPKG_TOPDIR}/tools/print-cores.sh
    remove_stale_files
}
trap finish EXIT

function check_health () {
    echo "======> Initiating launch sequence"
    echo "Please wait until all processes (${SIM_PROCESSES[@]}) are up"
    # loop for SETUP_WAIT_TIME seconds and break if all processes are UP
    counter=$SETUP_WAIT_TIME
    while [ $counter -gt 0 ]; do
        sleep 1
        ((counter--))
        for process in ${SIM_PROCESSES[@]}; do
            if ! pgrep -x $process > /dev/null
            then
                # continue while loop if process isn't up yet
                continue 2
            fi
        done
        echo "======> All systems go! Let's punch it"
        return
    done
    ps -ef > $PDSPKG_TOPDIR/health.log
    echo "ERR: Not all processes in (${SIM_PROCESSES[@]}) are up - Please check $PDSPKG_TOPDIR/health.log"
    exit 1
}

function setup_env () {
    sudo mkdir -p /var/log/pensando/ /obfl/ /sysconfig/config0/
    # TODO Remove this once agent code is fixed
    # Create dummy device.conf - agent is trying to update it when device object is updated.
    # Without this, pdsagent crashes since config file is not found.
    sudo touch /sysconfig/config0/device.conf
}

function setup () {
    # Cleanup of previous run if required
    stop_processes
    stop_model
    # remove stale files from older runs
    remove_stale_files
    remove_logs
    # setup env
    setup_env
}
setup

if [ $PIPELINE == 'artemis' ];then
    export AGENT_TEST_HOOKS_LIB='libdolagenthooks.so'
fi

if [ $DRYRUN == 0 ]; then
    start_model
    start_processes
    check_health
fi
