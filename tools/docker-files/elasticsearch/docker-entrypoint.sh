#!/bin/bash

# source and set env vars
echo "Sourcing mgmt_env.sh..."
source /usr/share/elasticsearch/mgmt_env.sh

# Keeping this for debuggability to catch race conditions if any
env network.host=$PENS_MGMT_IP env

echo "Setting permissions for data volume..."
chown -R 1000:1000 /usr/share/elasticsearch/data

# set network.host from PENS_MGMT_IP and launch elastic's entry script
env network.host=$PENS_MGMT_IP /usr/local/bin/docker-entrypoint.sh

