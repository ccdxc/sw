#! /bin/bash

export PIPELINE=apulu
export NICDIR=`pwd`

# initial setup
source ${NICDIR}/apollo/test/tools/setup_gtests.sh
setup

# run all gtests

if [[ "$1" ==  --coveragerun ]]; then
    # run sdk tests for code coverage
    run_sdk_gtest
fi

run_gtest scale LOG="/dev/null" CFG=" -i ${NICDIR}/apollo/test/scale/${PIPELINE}/scale_cfg.json "
run_gtest device
run_gtest vpc
run_gtest subnet
run_gtest route LOG="/dev/null"
run_gtest nh
run_gtest nh_group
run_gtest vnic
run_gtest tep
run_gtest if
run_gtest mapping LOG="/dev/null"
run_gtest mirror_session
run_gtest svc_mapping

# end of script
clean_exit
