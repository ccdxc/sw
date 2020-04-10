#! /bin/bash

set -ex

export OPERD_REGIONS=$CONFIG_PATH/operd-regions.json
OPERD_REGION='upgradelog'
USER=$(stat -c '%U' ${PDSPKG_TOPDIR}/apollo/test/tools/setup_upgrade_gtests.sh)

function upg_operd_init() {
    rm -f $CONFIG_PATH/operd-regions.json
    ln -s $CONFIG_PATH/$PIPELINE/operd-regions.json $CONFIG_PATH/operd-regions.json
}

function upg_pipeline_init() {
    rm -f ${CONFIG_PATH}/pipeline.json
    ln -s ${CONFIG_PATH}/${PIPELINE}/pipeline.json ${CONFIG_PATH}/pipeline.json
}


function upg_remove_logs() {
    rm -f ${PDSPKG_TOPDIR}/*log* ${PDSPKG_TOPDIR}/core*
}

function upg_init() {
    upg_operd_init
    upg_pipeline_init
    upg_remove_logs
}

function upg_setup() {
    mkdir -p $CONFIG_PATH/gen/
    rm -rf $CONFIG_PATH/gen/upgrade*.json
    cp $1 $CONFIG_PATH/gen/upgrade_graceful.json
}

function upg_finish() {
    echo "===== Collecting logs for $1 ====="
    for f in "/tmp/upgrade_*.log"; do
        if [ -e "$f" ];then
            cp $f ${PDSPKG_TOPDIR}
        fi
    done
    operdctl dump $OPERD_REGION > upgrade.log
    ${PDSPKG_TOPDIR}/apollo/test/tools/savelogs.sh
    rm -f $CONFIG_PATH/operd-regions.json
    rm -f ${CONFIG_PATH}/pipeline.json
}

function checkout_master() {
    cd ${PDSPKG_TOPDIR}/..
    git status
    if [[ $USER == "root" ]];then
        #sudo -u $USER git reset --hard HEAD
        git config --global user.email "team@pensando.io"
        git config --global user.name "team"
        git commit -am \"tmp\"
    fi
    sudo -u $USER git checkout -b origmaster origin/master
    git --no-pager log -2
    sudo -u $USER git submodule update --init --recursive
    rm -rf $BUILD_DIR
    sudo -u $USER bash ./scripts/pull-assets.sh
    make ws-tools
    cd ${PDSPKG_TOPDIR}
    mkdir -p $MASTER_TOPDIR
    rm -rf $MASTER_TOPDIR/*
}

function checkout_pr() {
    cd ${PDSPKG_TOPDIR}/..
    # switch to PR branch from master
    BRANCH=$(git rev-parse --abbrev-ref HEAD)
    if [[ "$BRANCH" != "origmaster" ]]; then
        cd ${PDSPKG_TOPDIR}
        return
    fi
    sudo -u $USER git checkout -
    git --no-pager log -2
    sudo -u $USER git submodule update --init --recursive
    rm -rf $BUILD_DIR
    sudo -u $USER bash ./scripts/pull-assets.sh
    make ws-tools
    cd ${PDSPKG_TOPDIR}
}

function is_modified() {
    git show --pretty="" --name-only HEAD | grep -q $1
    if [ $? == 0 ];then
        echo "file $1 modified"
        return 1
    else
        echo "file $1 not modified"
        return 0
    fi
}

function upg_wait_for_pdsagent() {
    NICMGR_FILE="$PDSPKG_TOPDIR/nicmgr.log"
    counter=600
    while [ $counter -gt 0 ]
    do
        if [ -f "$NICMGR_FILE" ]; then
            if grep -q "cpu_mnic0: Skipping MNIC device" $NICMGR_FILE; then
                echo "pds-agent is up"
                break
            fi
        fi
        echo "Waiting for pds-agent to be up, count - $counter"
        sleep 1
        counter=$(( $counter - 1 ))
    done
}
