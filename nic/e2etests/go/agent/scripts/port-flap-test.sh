#!/usr/bin/env bash
#set -x
# Default flap count is 5
FLAP_COUNT="${1:-5}"
i=0
set -e

function port_action
{
    action=$1
    curl -X PUT -d '{"kind":"Port","meta":{"name":"port1","tenant":"default","namespace":"default"},"spec":{"speed":"SPEED_100G","type":"TYPE_ETHERNET","breakout-mode":"BREAKOUT_NONE","admin-status":"'"$action"'","lanes":4}}' localhost:9007/api/system/ports/default/default/port1
    sleep 1
    curl -X PUT -d '{"kind":"Port","meta":{"name":"port5","tenant":"default","namespace":"default"},"spec":{"speed":"SPEED_100G","type":"TYPE_ETHERNET","breakout-mode":"BREAKOUT_NONE","admin-status":"'"$action"'","lanes":4}}' localhost:9007/api/system/ports/default/default/port5
    sleep 1
    curl -X PUT -d '{"kind":"Port","meta":{"name":"port9","tenant":"default","namespace":"default"},"spec":{"speed":"SPEED_100G","type":"TYPE_ETHERNET","breakout-mode":"BREAKOUT_NONE","admin-status":"'"$action"'","lanes":4}}' localhost:9007/api/system/ports/default/default/port9

}
until (( "$i" == "$FLAP_COUNT" ))
do
    echo "Admin DOWN all ports. Iteration $i..."
    port_action "DOWN"

    echo "Admin UP all ports..."
    port_action "UP"

    i=$((i + 1))
done

echo "Port Statuses after flap test"
curl localhost:9007/api/system/ports/