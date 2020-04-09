#! /bin/bash
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
export ASIC="${ASIC:-capri}"
export NICDIR=`dirname $ABS_TOOLS_DIR`
export DOLDIR=`readlink -f ${NICDIR}/../dol/`

rm -f ${NICDIR}/nic_sanity_logs.tar.gz

if ls ${NICDIR}/core.* 1> /dev/null 2>&1; then
    echo "waiting for the core to be produced"
    sleep 60
fi

tar cvzf ${NICDIR}/nic_sanity_logs.tar.gz -P --ignore-failed-read \
    ${NICDIR}/core.* ${NICDIR}/conf/gen/mpu_prog_info.json \
    ${NICDIR}/*log* \
    ${DOLDIR}/*log* \
    /root/naples/data/logs/* \
    *.log.* \
    /tmp/hal_techsupport/* \
    ${NICDIR}/build/x86_64/iris/${ASIC}/valgrind/*
