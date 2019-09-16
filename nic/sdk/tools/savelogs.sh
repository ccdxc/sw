#! /bin/bash -e

TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
export SDKDIR=`dirname $ABS_TOOLS_DIR`

rm -f ${SDKDIR}/sdk_sanity_logs.tar.gz

tar cvzf ${SDKDIR}/sdk_sanity_logs.tar.gz -P --ignore-failed-read \
    ${SDKDIR}/*log* \
    ${SDKDIR}/*.log.* \
    ${SDKDIR}/memhash_test.log \
    ${SDKDIR}/slhash_test.log \
    ${SDKDIR}/sltcam_test.log
