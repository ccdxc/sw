#! /bin/bash -e

CUR_DIR=$( readlink -f $( dirname $0 ) )

CMDARGS=""
LOGDIR=""
LOGSWITCH="--log-dir"
while (( "$#" )); do
    if [[ "$1" == $LOGSWITCH ]]; then
        if [[ -z $2 ]]; then
          echo "Log dir not specified"
          exit
        fi
        LOGDIR="$2"
        shift 2
    else
        CMDARGS+="$1 "
        shift
    fi
done

source $CUR_DIR/../setup_env_sim.sh apulu $LOGDIR

# mount hugetlbfs
mkdir -p /dev/hugepages
mount -t hugetlbfs nodev /dev/hugepages || { echo "Failed to mount hugetlbfs"; }

# set the dpdk sim application id for hitless upgrade
source ${PDSPKG_TOPDIR}/sdk/upgrade/core/upgmgr_core_base.sh
dom=$( upgmgr_init_domain )
if [[ $dom == $UPGRADE_DOMAIN_B ]];then
    export DPDK_SIM_APP_ID=2
fi

echo "Starting Agent: `date +%x_%H:%M:%S:%N`"
rm -f $PDSPKG_TOPDIR/conf/pipeline.json
ln -s $PDSPKG_TOPDIR/conf/apulu/pipeline.json $PDSPKG_TOPDIR/conf/pipeline.json
$GDB $BUILD_DIR/bin/pdsagent -c hal.json $CMDARGS 2>&1
#$VALGRIND $BUILD_DIR/bin/pdsagent -c hal.json $CMDARGS 2>&1
