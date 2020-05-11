#!/bin/bash

SCRIPTPATH="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
source ${SCRIPTPATH}/validate_options.sh 2>&1 > /dev/null

echo "firmware = ${fw}"
echo "stage = ${stage}"
echo "hook_type = ${hook_type}"
echo "response = ${response}"

echo "Love what you do ! but exit with failure "
exit 1
