#!/bin/bash

echo "Setting permissions for data volume..."
chown -R 1000:1000 /usr/share/elasticsearch/data
echo "Setting permissions for auth config volumes..."
chown -R 1000:1000 /usr/share/elasticsearch/config/auth-node
chown -R 1000:1000 /usr/share/elasticsearch/config/auth-https
chown -R 1000:1000 /var/log/pensando/elastic

# calculate heap size
mem_avail=$(awk '/MemAvailable/ {print $2}' /proc/meminfo)
mem_avail_in_gb=$(( mem_avail / (1024 * 1024) ))
heap_opts="-Xms1g -Xmx1g"
if (( mem_avail_in_gb < 64 ))
then
    heap_size=$(( mem_avail_in_gb / 3))
    if (( heap_size != 0 )); then
        heap_opts=$(echo "-Xms${heap_size}g -Xmx${heap_size}g")
    fi
else
    heap_opts="-Xms32g -Xmx32g"
fi

# set heap size options
echo "export ES_JAVA_OPTS=\"$heap_opts\"" >> /usr/share/elasticsearch/mgmt_env.sh

# source and set env vars
echo "Sourcing mgmt_env.sh..."
source /usr/share/elasticsearch/mgmt_env.sh

env discovery.zen.minimum_master_nodes=$ELASTIC_MIN_MASTERS env

if [[ ! -z "$PENS_MGMT_IP" ]]
then
    # Keeping this for debuggability to catch race conditions if any
    env network.host=$PENS_MGMT_IP env
    env network.bind_host=$PENS_MGMT_IP env
    env network.publish_host=$PENS_MGMT_IP env
    env http.publish_host=$PENS_MGMT_IP env

    # set network.host from PENS_MGMT_IP and launch elastic's entry script
    env network.host=$PENS_MGMT_IP network.publish_host=$PENS_MGMT_IP network.bind_host=$PENS_MGMT_IP http.publish_host=$PENS_MGMT_IP discovery.zen.minimum_master_nodes=$ELASTIC_MIN_MASTERS /usr/local/bin/docker-entrypoint.sh
else
    env discovery.zen.minimum_master_nodes=$ELASTIC_MIN_MASTERS /usr/local/bin/docker-entrypoint.sh
fi
