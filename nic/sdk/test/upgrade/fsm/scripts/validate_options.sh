#!/bin/bash

fw=""
stage=""
hook_type=""
response=""
valid_response="ok, critical, fail, noresponse"
valid_stage="UPG_STAGE_COMPAT_CHECK, UPG_STAGE_START, UPG_STAGE_BACKUP, \
UPG_STAGE_PREPARE, UPG_STAGE_SYNC, UPG_STAGE_PREP_SWITCHOVER, \
UPG_STAGE_SWITCHOVER, UPG_STAGE_READY, UPG_STAGE_RESPAWN, \
UPG_STAGE_ROLLBACK, UPG_STAGE_REPEAL, UPG_STAGE_FINISH"

function usage() {
    echo "Usage: $0 -f <firmware> -s <stage> -t PRE " 1>&2;
    echo "Usage: $0 -f <firmware> -s <stage> -t POST -r <ok|fail|critical|no-response>" 1>&2;
    echo
    echo "Valid stages are     : ${valid_stage}"
    echo "Valid responses are  : ${valid_response}"

    exit 1;
}

while getopts ":f:s:t:r:" o; do
    case "${o}" in
        f)
            fw=${OPTARG}
            ;;
        s)
            stage=${OPTARG}
            ;;
        t)
            hook_type=${OPTARG}
            [[ "${hook_type}" == "PRE" ]] || [[ "${hook_type}" == "POST" ]] || usage
            ;;
        r)
            response=${OPTARG}
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

if [ -z "${fw}" ] || [ -z "${stage}" ] || [ -z "${hook_type}" ] ; then
    usage
fi

if [ "${hook_type}" == "POST" ] ; then
    if [ -z "${response}" ]; then
        usage
    fi
fi

if [ "${hook_type}" == "PRE" ] ; then
    if [ ! -z "${response}" ]; then
        usage
    fi
fi

valid=false
SAVED_IFS=${IFS}
IFS=','
read -a stage_arr <<< ${valid_stage}
for (( n=0; n < ${#stage_arr[*]}; n++))
do
  if [ "${stage}" == "${stage_arr[n]}" ];then
      valid=true
      break
  fi
done
IFS=${SAVED_IFS}
[[ ${valid} ]] || usage

valid=false
SAVED_IFS=${IFS}
IFS=','
read -a rsp_arr <<< ${valid_response}
for (( n=0; n < ${#rsp_arr[*]}; n++))
do
  if [ "${response}" == "${rsp_arr[n]}" ];then
      valid=true
      break
  fi
done
IFS=${SAVED_IFS}
[[ ${valid} ]] || usage

