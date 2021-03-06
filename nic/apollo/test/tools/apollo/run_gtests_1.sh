#! /bin/bash

export PIPELINE=apollo
export NICDIR=`pwd`

# initial setup
source ${NICDIR}/apollo/test/tools/setup_gtests.sh
setup

# run all gtests
if [[ "$1" ==  --coveragerun ]]; then
    # run sdk tests for code coverage
    run_sdk_gtest
fi

run_gtest scale LOG="/dev/null" CFG=" -i ${NICDIR}/apollo/test/scale/scale_cfg.json "
run_gtest device
run_gtest vpc
run_gtest subnet
run_gtest route_table LOG="/dev/null"
run_gtest vnic
run_gtest tep
# run_gtest mapping LOG="/dev/null"
run_gtest mirror_session

# end of script
clean_exit
