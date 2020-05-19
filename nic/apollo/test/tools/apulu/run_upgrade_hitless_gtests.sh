#! /bin/bash

export PIPELINE=apulu
CUR_DIR=$( readlink -f $( dirname $0 ) )

# initial setup
source ${CUR_DIR}/../setup_gtests.sh
setup

# run all upg gtests

if [[ "$1" ==  --coveragerun ]]; then
    # run sdk tests for code coverage
    run_sdk_gtest
fi

run_gtest nh_group_upg
run_gtest mapping_upg

# end of script
clean_exit
