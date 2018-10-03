#! /bin/bash
rm -f /sw/nic/nic_sanity_logs.tar.gz
tar cvzf /sw/nic/nic_sanity_logs.tar.gz -P --ignore-failed-read \
    /sw/nic/core.* /sw/nic/gen/capri_loader.conf \
    /sw/nic/*log* \
    /sw/dol/*log* \
    /root/naples/data/logs/* \
    *.log.* \
    /tmp/hal_techsupport/*
