#!/bin/bash

SCRIPTPATH="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
source ${SCRIPTPATH}/validate_options.sh 2>&1 > /dev/null

echo "stage = ${stage}"
echo "hook_type = ${hook_type}"
echo "response = ${response}"

echo "All is well ! Go Corona Go"
