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
if (( mem_avail_in_gb < 32 ))
then
    heap_size=$(( mem_avail_in_gb / 8))
    if (( heap_size != 0 )); then
        heap_opts=$(echo "-Xms${heap_size}g -Xmx${heap_size}g")
    fi
else
    #
    # The standard recommendation is to give 50% of the available memory to Elasticsearch heap,
    # while leaving the other 50% free. It won’t go unused; Lucene will happily gobble up whatever is left over.
    # Since there are other processes running on the venice nodes(citadel, etcd, etc), we are using this formula to
    # arrive at a reasonable heap size.
    # e.g. 32G -> 12G (es heap), 40G -> 15G, 64G -> 24G
    #
    heap_size=$((3/8) * mem_avail_in_gb)
    if (( heap_size > 32 )); then
        # try to avoid crossing the 32 GB heap boundary. It wastes memory, reduces CPU performance,
        # and makes the GC struggle with large heaps.
       heap_size=32
    fi
    heap_opts=$(echo "-Xms${heap_size}g -Xmx${heap_size}g")
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
    exec env network.host=$PENS_MGMT_IP network.publish_host=$PENS_MGMT_IP network.bind_host=$PENS_MGMT_IP http.publish_host=$PENS_MGMT_IP \
        discovery.zen.minimum_master_nodes=$ELASTIC_MIN_MASTERS /usr/local/bin/docker-entrypoint.sh
else
    exec env discovery.zen.minimum_master_nodes=$ELASTIC_MIN_MASTERS /usr/local/bin/docker-entrypoint.sh
fi