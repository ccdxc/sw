#! /bin/bash

CUR_DIR=$( readlink -f $( dirname $0 ))
# run fsm test
$CUR_DIR/run_upgrade_fsm_test.sh
[[ $? -ne 0 ]] && echo "FSM test failed!" && exit 1
$CUR_DIR/run_upgrade_graceful_test.sh
[[ $? -ne 0 ]] && echo "Graceful test failed!" && exit 1
$CUR_DIR/run_upgrade_hitless_gtests.sh
[[ $? -ne 0 ]] && echo "Hitless gtest failed!" && exit 1
exit 0
