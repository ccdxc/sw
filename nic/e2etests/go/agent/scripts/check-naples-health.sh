#!/usr/bin/env bash

# check_for_naples_health checks if the naples-v1 container is unhealthy. If it exceeds MAX_RETRIES it will exit 1
NAPLES_CONTAINER_NAME="naples-v1"
MAX_RETRIES=128
NAPLES_HEALTHY=-1
health="unhealthy"
i=MAX_RETRIES
echo "Checking for NAPLES Sim Container Health Start: `date +%x_%H:%M:%S:%N`"
until (( NAPLES_HEALTHY == 0 )) || (( i == 0 ))
do
	echo "Checking if the naples container is healthy. Sleeping for 10 seconds..."
	sleep 10
	health=$(docker inspect -f '{{.State.Health.Status}}' "$NAPLES_CONTAINER_NAME")
	if [ "$health" == "healthy" ]; then
	    echo "NAPLES Container is healthy"
	    NAPLES_HEALTHY=0
	fi
	let "i--"
done

if [ "$i" -eq "0" ]; then
	echo "NAPLES Container is unhealthy"
	cat /root/naples/data/logs/start-naples.log
	docker exec "$NAPLES_CONTAINER_NAME" bash -c /naples/nic/tools/print-cores.sh
	docker ps
	exit 1
fi
echo "Checking for NAPLES Sim Container Health End: `date +%x_%H:%M:%S:%N`"
