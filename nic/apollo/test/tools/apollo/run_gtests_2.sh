#! /bin/bash

export PIPELINE=apollo
export NICDIR=`pwd`

# initial setup
source ${NICDIR}/apollo/test/tools/setup_gtests.sh
setup

# run all gtests
run_gtest policy LOG="/dev/null"
run_gtest rte_bitmap

# end of script
clean_exit