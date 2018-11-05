#! /bin/bash
rm -f /sw/iota/iota_sanity_logs.tar.gz
tar cvzf /sw/iota/iota_sanity_logs.tar.gz -P --ignore-failed-read \
    /sw/iota/logs \
    /sw/iota/core.* \
    /sw/iota/*.log
