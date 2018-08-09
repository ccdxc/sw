#!/usr/bin/env bash

# check_for_naples_health checks if the naples-v1 container is unhealthy. If it exceeds MAX_RETRIES it will exit 1
# MAX_RETRIES guards the maximum time we wait for NAPLES Container to change state to healthy.
# It waits for a total of 2**(MAX_RETRIES) - 1 seconds
NAPLES_CONTAINER_NAME="naples-v1"
MAX_RETRIES=16
NAPLES_HEALTHY=-1
health="unhealthy"
i=0
echo "Checking for NAPLES Sim Container Health Start: `date +%x_%H:%M:%S:%N`"
until (( NAPLES_HEALTHY == 0 )) || (( i == MAX_RETRIES ))
do
	timeout="$((2 ** i))"
	echo "Checking if the naples container is healthy. Sleeping for $timeout seconds..."
	sleep "$timeout"
	health=$(docker inspect -f '{{.State.Health.Status}}' "$NAPLES_CONTAINER_NAME")
	if [ "$health" == "healthy" ]; then
	    echo "NAPLES Container is healthy"
	    NAPLES_HEALTHY=0
	fi
	let "i++"
done

if [ "$i" -eq "$MAX_RETRIES" ]; then
	echo "NAPLES Container is unhealthy"
	cat /root/naples/data/logs/start-naples.log
	docker exec "$NAPLES_CONTAINER_NAME" bash -c /naples/nic/tools/print-cores.sh
	docker ps
	exit 1
fi
echo "Checking for NAPLES Sim Container Health End: `date +%x_%H:%M:%S:%N`"
