#!/bin/bash

echo "Stopping NAPLES simulation ..."

# stop any running container
export NAPLES_INSTALL_DIR=$HOME/naples/v1
docker rm -f naples-v1
docker rmi -f naples:v1

echo "Stopped NAPLES simulation ..."
