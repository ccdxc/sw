#! /bin/bash

export PIPELINE=apulu
CUR_DIR=$( readlink -f $( dirname $0 ) )

# initial setup
source ${CUR_DIR}/../setup_gtests.sh
setup

# run all gtests
run_valgrind_gtest batch
run_valgrind_gtest device
run_valgrind_gtest tep
run_valgrind_gtest scale CFG=" -i ${PDSPKG_TOPDIR}/apollo/test/scale/${PIPELINE}/scale_cfg_1vpc_valgrind.json"

# end of script
clean_exit
