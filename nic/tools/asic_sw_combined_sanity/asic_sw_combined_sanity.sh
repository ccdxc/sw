#! /bin/bash

source ~/.bashrc

function apply_patches()
{
    DIFFS_DIR=$1
    if [ -d $DIFFS_DIR ]; then
        for dfile in `ls -1 $DIFFS_DIR/*`
        do
            echo "Applying Diff File: $dfile"
            git apply --reject $dfile || exit 1
        done
    fi
}

###################
# SW REPO exports
###################

export WORKSPACE=/local/jenkins/workspace/$JOB_BASE_NAME
export GIT_REPO="https://github.com/pensando/sw"
export GIT_BRANCH="master"
export SW_REPO_DIR=$WORKSPACE/src/github.com/pensando

###################
# ASIC REPO exports
###################

export ASIC_GIT_REPO="https://github.com/pensando/asic"
export ASIC_REPO_DIR=$WORKSPACE/src/github.com/pensando/sw/nic/sdk/asic_repo
export ASIC_EXPORT_DIR=$WORKSPACE/src/github.com/pensando/sw/nic/sdk/third-party/asic

################
# Setup SW repo
################

function setup_sw_repo () {
    # create dirs
    rm -rf $WORKSPACE || exit 1
    mkdir -p $SW_REPO_DIR || exit 1
    cd $SW_REPO_DIR

    # clone SW repo
    git clone $GIT_REPO --recursive || exit 1

    cd $SW_REPO_DIR/sw || exit 1

    # checkout SW branch
    git checkout $GIT_BRANCH || exit 1

    set_go ASIC_SW_COMBINED_SANITY

    # submodule init
    git submodule update --init --recursive || exit 1

    # assets
    make pull-assets || exit 1

    # GO paths
    make ws-tools || exit 1
}

###########################
# Setup ASIC repo under SDK
###########################

function setup_asic_repo () {
    # create dirs
    mkdir -p $ASIC_REPO_DIR

    cd $ASIC_REPO_DIR

    # clone ASIC repo
    git clone $ASIC_GIT_REPO --recursive || exit 1

    cd $ASIC_REPO_DIR/asic

    # update submodules for asic repo
    git submodule update --init --recursive

    # checkout ASIC SHA if applicable
    if [ "$DONT_UPDATE_ASSET_LABEL" = "true" ]; then
        echo "Not updating ASSET_LABEL"
    else
        git fetch origin $ASIC_BRANCH || exit 1
        git checkout $ASSET_LABEL || exit 1

        # update submodules after checking out asset label
        git submodule update --init --recursive
    fi
}

function export_asic_files () {
    cd $SW_REPO_DIR/sw
    for file in `cat nic/tools/asic_sw_combined_sanity/asic_headers.txt`
    do
        destdir=`dirname $file`
        cp $ASIC_REPO_DIR/asic/$file $ASIC_EXPORT_DIR/$destdir/
    done
}

function export_asic_libs () {
    cd $SW_REPO_DIR/sw
    mkdir -p nic/sdk/third-party/asic/aarch64/lib/
    mkdir -p nic/sdk/third-party/asic/x86_64/lib/
    cp nic/build/aarch64/iris/lib/libsdkcapri_csrint.so nic/sdk/third-party/asic/aarch64/lib/
    cp nic/build/x86_64/iris/lib/libsdkcapri_csrint.so  nic/sdk/third-party/asic/x86_64/lib/
}

########################
# Apply software patches
########################

function apply_sw_patches () {
    # Patch for SW repo
    cd $SW_REPO_DIR/sw
    apply_patches "/var/lib/jenkins/diffs/ASIC_SW_COMBINED_SANITY/sw"

    # Patch for ASIC repo
    cd $ASIC_REPO_DIR/asic
    apply_patches "/var/lib/jenkins/diffs/ASIC_SW_COMBINED_SANITY/asic"
}

function print_vars () {
    echo "-------------------------------------------------"
    echo "ENVIRONMENT VARIABLES"
    echo "-------------------------------------------------"
    echo "WORKSPACE:               $WORKSPACE"
    echo "GIT_REPO:                $GIT_REPO"
    echo "GIT_BRANCH:              $GIT_BRANCH"
    echo "SW_REPO_DIR:             $SW_REPO_DIR"
    echo "ASIC_GIT_REPO:           $ASIC_GIT_REPO"
    echo "ASIC_REPO_DIR:           $ASIC_REPO_DIR"
    echo "ASSET_LABEL:             $ASSET_LABEL"
    echo "DONT_UPDATE_ASSET_LABEL: $DONT_UPDATE_ASSET_LABEL"
    echo "-------------------------------------------------"
}

function build_asic () {
    cd $SW_REPO_DIR/sw/nic
    ./build_model.sh     || exit 1 
    ./build_pb_config.sh || exit 1 
}

function build_sw () {
    cd $SW_REPO_DIR/sw
    make ws-tools

    # Build NIC targets
    cd $SW_REPO_DIR/sw/nic

    # rename module.mk for asic files
    mv sdk/platform/capri/csrint/module.mk.asic_sw_combined_sanity sdk/platform/capri/csrint/module.mk

    make clean || exit 1
    make ARCH=aarch64 PLATFORM=hw clean || exit 1

    make libsdkcapri_csrint.so || exit 1
    make ARCH=aarch64 PLATFORM=hw libsdkcapri_csrint.so || exit 1

    # rename module.mk for asic files
    mv sdk/platform/capri/csrint/module.mk sdk/platform/capri/csrint/module.mk.asic_sw_combined_sanity

    export_asic_libs

    cd $SW_REPO_DIR/sw/nic

    make || exit 1
    make ARCH=aarch64 PLATFORM=hw || exit 1
}

print_vars
setup_sw_repo
setup_asic_repo
apply_sw_patches
export_asic_files

# For ASIC repo, if ASSET_LABEL is not specified, use the SHA of the top commit
cd $ASIC_REPO_DIR/asic
SHA=`git rev-parse --short HEAD`
if [ "$ASSET_LABEL" = "" ]; then
    ASSET_LABEL=$SHA
fi

echo "ASSET_LABEL: $ASSET_LABEL"

build_asic
build_sw

# cleanup the patches
rm -rf /var/lib/jenkins/diffs/ASIC_SW_COMBINED_SANITY/sw.done
rm -rf /var/lib/jenkins/diffs/ASIC_SW_COMBINED_SANITY/asic.done
mv /var/lib/jenkins/diffs/ASIC_SW_COMBINED_SANITY/sw   /var/lib/jenkins/diffs/ASIC_SW_COMBINED_SANITY/sw.done
mv /var/lib/jenkins/diffs/ASIC_SW_COMBINED_SANITY/asic /var/lib/jenkins/diffs/ASIC_SW_COMBINED_SANITY/asic.done

cd $SW_REPO_DIR/sw

# update ASSET_LABEL in minio/VERSIONS
perl -i -pe "s/asic_libs .*/asic_libs ${ASSET_LABEL}/g; s/ncc_tools .*/ncc_tools ${ASSET_LABEL}/g; s/pbc_config .*/pbc_config ${ASSET_LABEL}/g;" minio/VERSIONS

# increment the third-party version in minio/VERSIONS
sed -ri 's/(third_party_libs 1.1.)([0-9]+)/echo "\1$((\2+1))"/ge' minio/VERSIONS

THIRD_PARTY_ASSET_LABEL=`cat minio/VERSIONS | grep third_party_libs | awk '{print $2}'`
echo "THIRD_PARTY_ASSET_LABEL: $THIRD_PARTY_ASSET_LABEL"

# upload to assets
tar cvz $(cat minio/asic_libs.txt)  | asset-upload -f asic_libs  ${ASSET_LABEL} /dev/stdin
tar cvz $(cat minio/ncc_tools.txt)  | asset-upload -f ncc_tools  ${ASSET_LABEL} /dev/stdin
tar cvz $(cat minio/pbc_config.txt) | asset-upload -f pbc_config ${ASSET_LABEL} /dev/stdin
tar cvz $(cat minio/third_party_libs.txt) | asset-upload -f third_party_libs ${THIRD_PARTY_ASSET_LABEL} /dev/stdin

# Prepare to push the changes
git remote add upstream $CS_REMOTE || exit 1
git fetch upstream                 || exit 1
echo "Branch = $CS_BRANCH"
git add -u || exit 1
git checkout -b $CS_BRANCH              || exit 1 
git commit -m "Update asic ptr $SHA"    || exit 1
git push -f upstream $CS_BRANCH         || exit 1

echo "$CS_BRANCH" > /vol/dump/jenkins/asic_sw_combined_sanity.branch
echo "$SHA"       > /vol/dump/jenkins/asic_sw_combined_sanity.label
