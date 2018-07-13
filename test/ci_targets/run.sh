#!/bin/bash
cd /tmp/git/pensando/sw
make $1
if [ $? -ne 0 ]; then
  mkdir -p /tmp/ClusterLogs
  for node in $(docker ps -f label=pens-dind --format '{{.Names}}'); do
    docker cp  $node:/var/log/pensando /tmp/ClusterLogs/$node.logs
  done
  tar cvfz /tmp/clusterlogs.tgz /tmp/ClusterLogs
  exit -1
fi
