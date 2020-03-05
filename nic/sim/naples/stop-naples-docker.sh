#!/bin/bash

VER=v1
echo "Stopping NAPLES container ..."
docker rm -f naples-v1
if [ $? -eq 0 ]; then
    echo "Stopped NAPLES container ..."
else
    echo "NAPLES container not runnning"
fi

# remove any dangling images
docker images purge
#docker rmi -f $(docker images -q -f dangling=true)
docker rmi -f pensando/naples:$VER

exit 0
