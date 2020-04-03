#!/bin/sh

ATHENA_APP_EN_FILE=/data/athena_app_en

if [ -f $ATHENA_APP_EN_FILE ]; then
    echo "Execing Athena App"
    exec taskset 1 //nic/tools/start-agent.sh
else
    echo "Skipping Execing of Athena App"
fi
