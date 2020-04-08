#!/bin/bash

if [ $# != 1 ];then
    echo "usage : fsm_test <bin_dir>"
    exit 1
fi

TEST_BIN_DIR=$1

CUR_DIR=$( readlink -f $( dirname $0 ))
TEST_BIN_NAME=fsm_test

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


CMD1="${TEST_BIN_DIR}/${TEST_BIN_NAME} -s svc1 -i 51"
CMD2="${TEST_BIN_DIR}/${TEST_BIN_NAME} -s svc2 -i 52"
#CMD2="${TEST_BIN_DIR}/${TEST_BIN_NAME} -s svc2 -i 52 -e fail -f backup"
#CMD3="${TEST_BIN_DIR}/${TEST_BIN_NAME} -s 'svc3' -i '53' -e 'critical' -f 'finish'"

create_svc_thread ${CMD1}
create_svc_thread ${CMD2}
#create_svc_thread ${CMD3}

wait

exit 0
