#!/bin/bash
export GOPATH="/import"
export E2E_CONFIG="test/e2e/cluster/tb_config_ci.json"
cd $GOPATH/src/github.com/pensando/sw
make $1
if [ $? -ne 0 ]; then
  for node in $(docker ps -f label=pens-dind --format '{{.Names}}'); do
    mkdir -p /tmp/ClusterLogs/$node/events
    docker cp  $node:/var/log/pensando /tmp/ClusterLogs/$node/logs
    docker exec $node mkdir -p /var/lib/pensando/events
    docker exec $node tar Ccf /var/lib/pensando/events - . | tar Cxf /tmp/ClusterLogs/$node/events -
    docker exec  $node journalctl -x --no-pager > /tmp/ClusterLogs/$node/logs/journalctl
  done
  for node in $(docker ps -f label=pens-naples --format '{{.Names}}'); do
    mkdir -p /tmp/ClusterLogs/$node/events
    docker cp  $node:/var/log/pensando /tmp/ClusterLogs/$node/logs
    docker exec $node mkdir -p /var/lib/pensando/events
    docker exec $node tar Ccf /var/lib/pensando/events - . | tar Cxf /tmp/ClusterLogs/$node/events -
  done
  tar cvfz /tmp/clusterlogs.tgz /tmp/ClusterLogs
  exit -1
fi
