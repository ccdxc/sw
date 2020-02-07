#! /bin/bash

export PIPELINE=athena
export NICDIR=`pwd`

# initial setup
source ${NICDIR}/apollo/test/tools/setup_gtests.sh
setup

# run all gtests

if [[ "$1" ==  --coveragerun ]]; then
    # run sdk tests for code coverage
    run_sdk_gtest
fi

run_gtest athena_vnic
run_gtest conntrack
run_gtest flow_cache
run_gtest flow_session

# end of script
clean_exit
