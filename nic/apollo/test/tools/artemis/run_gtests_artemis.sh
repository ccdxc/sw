#! /bin/bash

export PIPELINE=artemis
export NICDIR=`pwd`

# initial setup
source ${NICDIR}/apollo/test/tools/setup_gtests.sh
setup

# run all gtests
#run_gtest device
run_gtest vpc_peer
# run_gtest tep
# run_gtest nh
# run_gtest route LOG="/dev/null"
# run_gtest vnic
# run_gtest svc_mapping
run_gtest tag LOG="/dev/null"

# end of script
clean_exit
