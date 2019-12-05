#!/bin/bash

TOP=${TOP:-/go/src/github.com/pensando/sw}
VERSIONS_FILE=$TOP/scripts/module_versions.txt
GITMODULES=$TOP/.gitmodules

# Make sure all the submodules exist have a file in VERSIONS_FILE.
# buildroot is a special case
for submodule in `grep path $GITMODULES | sed 's/.*= \(.*\)/\1/'`; do
    grep -c $submodule $VERSIONS_FILE > /dev/null || \
        [[ $submodule == 'nic/buildroot' ]] || \
        echo "$submodule doesn't have a version in /sw/scripts/module_versions.txt"
done

# Read the module_versions.txt files and make sure the submodule versions match
while read -a LINE; do
    MODULE_PATH=${LINE[0]}
    VERSION=${LINE[1]}
    GIT_VERSION=`(cd $TOP/$MODULE_PATH; git rev-parse HEAD)`
    echo Checking $MODULE_PATH
    [[ $GIT_VERSION == $VERSION ]] || \
	{ echo $MODULE_PATH Version mismatch $GIT_VERSION != $VERSION; exit -1; }
done < $VERSIONS_FILE

# BUILDROOT has special case. We check directly with minio/VERSIONS
BUILDROOT_IS=`(cd $TOP/nic/buildroot; git rev-parse HEAD)`
BUILDROOT_SHOULD_BE=`grep buildroot $TOP/minio/VERSIONS  | awk '{ print $NF }'`

[[ $BUILDROOT_IS == $BUILDROOT_SHOULD_BE ]] || \
    { echo BUILDROOT $BUILDROOT_IS != $BUILDROOT_SHOULD_BE; exit -1; }

