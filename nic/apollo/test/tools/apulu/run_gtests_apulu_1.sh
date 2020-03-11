#! /bin/bash

export PIPELINE=apulu
CUR_DIR=$( readlink -f $( dirname $0 ) )

# initial setup
source ${CUR_DIR}/../setup_gtests.sh
setup

# run all gtests

if [[ "$1" ==  --coveragerun ]]; then
    # run sdk tests for code coverage
    run_sdk_gtest
fi

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
run_gtest ftl
run_gtest dhcp_relay
run_gtest scale LOG="/dev/null" CFG=" -i ${CUR_DIR}/../../scale/${PIPELINE}/scale_cfg.json "

# end of script
clean_exit
