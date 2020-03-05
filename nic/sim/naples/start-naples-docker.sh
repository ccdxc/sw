#!/bin/bash

VER=v1
NAPLES_DIR=$HOME/naples/data
AGENT_PORT=9007
HAL_PORT=50054
CONTAINER="pensando/naples:$VER"
DEF_LIFS=16

LONGOPTIONS="smartnic,qemu,lifs:"

PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTIONS --name "$0" -- "$@")
if [[ $? -ne 0 ]]; then
    # e.g. $? == 1
    #  then getopt has complained about wrong arguments to stdout
    exit 2
fi

eval set -- "$PARSED"
lifs=$DEF_LIFS
qemu=0
smartnic=0
while true; do
    case "$1" in
         --qemu)
            qemu=1
            shift
            ;;
         --smartnic)
            smartnic=1
            shift
            ;;
        -c|--lifs)
            lifs=$2
            shift 2
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "Programming error"
            exit 3
            ;;
    esac
done
# create a dir so we can mount logs etc. from NAPLES into this
mkdir -p $NAPLES_DIR

echo "Purging old/dangling images, if any ..."
# remove any dangling images
# docker rmi -f $(docker images -q -f dangling=true)
docker images purge
docker rmi -f pensando/naples:$VER

echo "Loading docker image into registry ..."
docker load --input naples-docker-$VER.tgz

if [ "$qemu" -eq 1 ]
  then
    echo "Running the NAPLES container in Qemu mode ..."
    docker run --rm -d --name naples-$VER --privileged -ti -p $AGENT_PORT:$AGENT_PORT  -p $HAL_PORT:$HAL_PORT --mount type=bind,source=$NAPLES_DIR,target=/naples/data -e WITH_QEMU=1 "$CONTAINER" 
else
    echo "Running the NAPLES container in stand-alone mode ..."
    docker run --rm -d --name naples-$VER --privileged -ti -p $AGENT_PORT:$AGENT_PORT  -p $HAL_PORT:$HAL_PORT --mount type=bind,source=$NAPLES_DIR,target=/naples/data -e MAX_LIFS=$lifs -e SMART_NIC_MODE=$smartnic "$CONTAINER"
fi

echo "NAPLES bring up completed"
