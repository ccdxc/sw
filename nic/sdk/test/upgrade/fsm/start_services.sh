#!/bin/bash

if [ $# == 3 ]; then
    TEST_BIN_DIR=$1
    SVC=$2
    SVC_ID=$3
elif [ $# == 5 ]; then
    TEST_BIN_DIR=$1
    SVC=$2
    SVC_ID=$3
    ERR_CODE=$4
    ERR_STAG=$5
else
    echo "usage : fsm_test <bin_dir> <svc> <id>"
    echo "usage : fsm_test <bin_dir> <svc> <id> <erro_code> <error_stage>"
    exit 1
fi

CUR_DIR=$( readlink -f $( dirname $0 ))
TEST_BIN_NAME=fsm_test

function create_svc_thread()
{

    local start_time=$(date +"%H:%M:%S")
    local end_time=""

    echo "Starting service : ($start_time)..."
    "$@" && time2=$(date +"%H:%M:%S") && \
        echo "Exiting $2 ($start_time--$time2)..."
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

if [ $# == 3 ];then
    CMD="${TEST_BIN_DIR}/${TEST_BIN_NAME} -s ${SVC} -i ${SVC_ID}"
elif [ $# == 5 ];then
    CMD="${TEST_BIN_DIR}/${TEST_BIN_NAME} -s ${SVC} -i ${SVC_ID} -e ${ERR_CODE} -f ${ERR_STAG}"
else
    echo "Something Wrong !"
fi

echo "Starting test service : ${CMD}"
create_svc_thread ${CMD}

wait
sleep 3
exit 0
