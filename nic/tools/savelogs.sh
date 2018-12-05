#! /bin/bash
rm -f /sw/nic/nic_sanity_logs.tar.gz

if ls /sw/nic/core.* 1> /dev/null 2>&1; then
    echo "waiting for the core to be produced"
    sleep 60
fi

tar cvzf /sw/nic/nic_sanity_logs.tar.gz -P --ignore-failed-read \
    /sw/nic/core.* /sw/nic/gen/capri_loader.conf \
    /sw/nic/*log* \
    /sw/dol/*log* \
    /root/naples/data/logs/* \
    *.log.* \
    /tmp/hal_techsupport/* \
    /sw/nic/build/x86_64/iris/valgrind/*
