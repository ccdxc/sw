#!/bin/sh

source /nic/tools/setup_env_hw.sh

# destination directory for techsupport
TS_DIR="/data/techsupport/"

TS_PREFIX="DSC_TechSupport_"
TS_SUFFIX=".tar.gz"

# generate a unique ID for the techsupport instance
TS_INSTANCE=`date +%Y-%m-%d_%H-%M-%S`

# construct filename for techsupport
TS_NAME="${TS_PREFIX}${TS_INSTANCE}${TS_SUFFIX}"

echo "Collecting Techsupport...Please wait"
$PDSPKG_TOPDIR/bin/techsupport -c ${CONFIG_PATH}/techsupport.json -d ${TS_DIR} -o ${TS_NAME} >/dev/null 2>&1
[[ $? -ne 0 ]] && echo "Failed to collect techsupport" && exit 1
echo "Techsupport collected at ${TS_DIR}/${TS_NAME}"
exit 0