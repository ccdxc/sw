#! /bin/bash

set -e

cd /sw/nic/obj/images
tar xvzf naples-release-v1.tgz
source /sw/nic/sim/naples/start-naples-docker.sh
NAPLES_CID=$(docker ps | grep -v CONTAINER | cut -d' ' -f1)
if [ "$NAPLES_CID" == "" ]; then
    "echo NAPLES container not found"
    exit 1
fi
echo "NAPLES container $NAPLES_CID found"

PROC_FOUND=$(docker top $NAPLES_CID | grep cap_model)
if [ "PROC_FOUND" == "" ]; then
    echo "Model not running"
    exit 1
fi
echo "Model running"

PROC_FOUND="$(docker top $NAPLES_CID | grep hal)"
if [ "PROC_FOUND" == "" ]; then
    echo "HAL not running"
    exit 1
fi
echo "HAL running"

# give sometime for HAL & model to initialize
sleep 60
PROC_FOUND="$(docker top $NAPLES_CID | grep netagent)"
if [ "PROC_FOUND" == "" ]; then
    echo "Agent not running"
    exit 1
fi
echo "Netagent running"

PROC_FOUND="$(docker top $NAPLES_CID | grep nic_infra_hntap)"
if [ "PROC_FOUND" == "" ]; then
    echo "HNTAP not running"
    exit 1
fi
echo "HNTAP running"
exit 0
