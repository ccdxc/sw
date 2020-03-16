#!/bin/bash
NAME="pdsupgtesteventsvc.bin"
CMD_NAME="pdsupgtestevent"

export PIPELINE=apulu
WORKING_DIR=$( readlink -f $( dirname $0 ))
source  ${WORKING_DIR}/../../../tools/setup_env_mock.sh $PIPELINE

BINDIR="${BUILD_DIR}/out/`echo ${NAME} | tr '.' '_'`"
SVC_BIN="${BINDIR}/${NAME}"
SVC1_STARTUP_CONFIG="${WORKING_DIR}/svc_1_init.json"
SVC2_STARTUP_CONFIG="${WORKING_DIR}/svc_2_init.json"
SVC3_STARTUP_CONFIG="${WORKING_DIR}/svc_3_init.json"
TEST_CONFIG="${WORKING_DIR}/test.json"


source
function at_term()
{
  echo "\n Interrupted !"
  echo "Stopping services .."
  pkill -P $$ > /dev/null 2>&1
  pgrep ${CMD_NAME} | xargs -n1 kill > /dev/null 2>&1
}

function at_start()
{
    local cmd=""
    cmd=`ps -aux | grep ${NAME} | \
        awk -F " " '{print $2}' | \
        xargs kill -9 > /dev/null 2>&1`
}

function at_exit()
{
    echo "Exit from main ..."
    #TODO: dump log
    #/bin/pgrep ${CMD_NAME} | xargs -n1 kill > /dev/null 2>&1
}

function die()
{
    echo "Errror:\"$0\" at line $BASH_LINENO"
}

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

LOG1="${WORKING_DIR}/$(log_file ${SVC1_STARTUP_CONFIG}).log"
LOG2="${WORKING_DIR}/$(log_file ${SVC2_STARTUP_CONFIG}).log"
LOG3="${WORKING_DIR}/$(log_file ${SVC3_STARTUP_CONFIG}).log"

CMD1="${SVC_BIN} -s ${SVC1_STARTUP_CONFIG} -t ${TEST_CONFIG}"
CMD2="${SVC_BIN} -s ${SVC2_STARTUP_CONFIG} -t ${TEST_CONFIG}"
CMD3="${SVC_BIN} -s ${SVC3_STARTUP_CONFIG} -t ${TEST_CONFIG}"


trap  at_term      SIGTERM
trap  at_term      SIGINT
trap  at_exit      EXIT
#trap die          ERR

create_svc_thread ${CMD1}
create_svc_thread ${CMD2}
#create_svc_thread ${CMD3}

wait

exit 0
