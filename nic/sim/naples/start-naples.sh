#!/bin/bash

#if [ "$1" = "" ]; then
    #echo "usage: $0 <installed-directory-path>"
    #exit;
#fi

export NAPLES_INSTALL_DIR=$HOME/naples/v1
docker run -d --name naples-v1 --privileged -ti --mount type=bind,source=$NAPLES_INSTALL_DIR,target=/naples/v1 naples:v1 /bin/bash -c /naples/v1/nic/tools/start-naples-sim.sh

# check if the container is successfully running etc.
# docker ps -a | grep naples-v1 (look for Exited or Running)
echo "NAPLES installation completed"
