#!/bin/bash
VER=v1
if [ "$NAPLES_DIR" = "" ]; then
    export NAPLES_DIR="/var/naples"
fi

LOGDIR=${NAPLES_DIR}/logs
mkdir -p $LOGDIR
exec > $LOGDIR/stop_naples_docker.log
exec 2>&1
set -x

echo "Stopping NAPLES container ..."
docker rm -f naples-sim
if [ $? -eq 0 ]; then
    echo "Stopped NAPLES container ..."
else
    echo "NAPLES container not runnning"
fi

set -e
# remove any dangling images
docker images purge
#docker rmi -f $(docker images -q -f dangling=true)
docker rmi -f naples:$VER

exit 0
