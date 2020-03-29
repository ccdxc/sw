#! /bin/bash

export PIPELINE=apulu
CUR_DIR=$( readlink -f $( dirname $0 ) )

# initial setup
source ${CUR_DIR}/../setup_gtests.sh
setup

# run all gtests
run_gtest batch
run_gtest policy_rule
run_gtest policy LOG="/dev/null"
run_gtest rte_bitmap

# end of script
clean_exit
