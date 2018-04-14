#!/bin/bash

echo "Uninstalling NAPLES simulation ..."

# stop any running container
./naples/v1/nic/tools/stop-naples.sh

# remove the container image
docker rmi -f naples:v1

export NAPLES_INSTALL_DIR=$HOME/naples/v1
# rm -rf $HOME/naples/v1/
