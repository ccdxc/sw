#!/bin/bash
export GOPATH="/import"
export E2E_CONFIG="test/e2e/cluster/tb_config_ci.json"
cd $GOPATH/src/github.com/pensando/sw
make $1
if [ $? -ne 0 ]; then
  mkdir -p /tmp/ClusterLogs
  for node in $(docker ps -f label=pens-dind --format '{{.Names}}'); do
    docker cp  $node:/var/log/pensando /tmp/ClusterLogs/$node.logs
  done
  for node in $(docker ps -f label=pens-naples --format '{{.Names}}'); do
    docker cp  $node:/var/log/pensando /tmp/ClusterLogs/$node.logs
  done
  tar cvfz /tmp/clusterlogs.tgz /tmp/ClusterLogs
  exit -1
fi
