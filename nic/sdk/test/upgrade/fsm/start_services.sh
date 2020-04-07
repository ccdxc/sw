#!/bin/bash

if [ $# != 1 ];then
    echo "usage : fsm_test <bin_dir>"
    exit 1
fi

TEST_BIN_DIR=$1

CUR_DIR=$( readlink -f $( dirname $0 ))
TEST_BIN_NAME=fsm_test
SVC1_STARTUP_CONFIG="${CUR_DIR}/svc_1_init.json"
SVC2_STARTUP_CONFIG="${CUR_DIR}/svc_2_init.json"
SVC3_STARTUP_CONFIG="${CUR_DIR}/svc_3_init.json"
TEST_CONFIG="${CUR_DIR}/test.json"
LOG_DIR=`pwd`

function create_svc_thread()
{

    local start_time=$(date +"%H:%M:%S")
    local end_time=""

    echo "Starting service : ($start_time)..."
    "$@" && time2=$(date +"%H:%M:%S") && \
        echo "Exiting $2 ($start_time--$time2)..." &
}

function log_file()
{
    local  retval
    retval=`grep "svc_name" ${1} | \
        awk -F ':' '{print $2}'| \
        cut -d',' -f-1`
    retval=`echo ${retval} | sed 's/\"//g'`
    retval=`echo ${retval} | sed 's/ //g'`
    echo "$retval"
}

LOG1="${LOG_DIR}/$(log_file ${SVC1_STARTUP_CONFIG}).log"
LOG2="${LOG_DIR}/$(log_file ${SVC2_STARTUP_CONFIG}).log"
LOG3="${LOG_DIR}/$(log_file ${SVC3_STARTUP_CONFIG}).log"

CMD1="${TEST_BIN_DIR}/${TEST_BIN_NAME} -s ${SVC1_STARTUP_CONFIG} -t ${TEST_CONFIG}"
CMD2="${TEST_BIN_DIR}/${TEST_BIN_NAME} -s ${SVC2_STARTUP_CONFIG} -t ${TEST_CONFIG}"
CMD3="${TEST_BIN_DIR}/${TEST_BIN_NAME} -s ${SVC3_STARTUP_CONFIG} -t ${TEST_CONFIG}"

create_svc_thread ${CMD1}
create_svc_thread ${CMD2}
#create_svc_thread ${CMD3}

wait

exit 0
