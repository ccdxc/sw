#!/bin/bash

exec > /start-naples.log
exec 2>&1

set -x

echo "Bringing up NAPLES services/processes ..."

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/nic/lib:/nic/lib64:/nic/usr/lib:/usr/lib

# serial-number is hardcoded in go code.
# nic/agent/nmd/state/rolloutif/smartnic_rollout.go
fru_base='{
    "manufacturing-date": "1539734400",
    "manufacturer": "PENSANDO SYSTEMS INC.",
    "product-name": "NAPLES 100",
    "serial-number": "SIM18440000",
    "part-number": "68-0003-02 01",
    "engineering-change-level": "00",
    "board-id": "1000",
    "num-mac-address": "24",
    "mac-address":'

mac_base="00:ae:cd:01:02:"
rand=$((RANDOM%100))


if [ -z "$SYSUUID" ]; then
    if (( $rand > 9 )); then
        mac='"00:ae:cd:01:02:'$rand'"'
    else
        mac='"00:ae:cd:01:02:0'$rand'"'
    fi
else
    mac='"'$SYSUUID'"'
fi

fru_final=$fru_base$mac"}"
echo $fru_final > /tmp/fru.json


echo "Starting sysmgr ...no datapth"
/nic/bin/sysmgr /nic/conf/sysmgr_no_datapath.json &

echo "NAPLES services/processes up and running ..."
while :; do
      sleep 300
done
exit 0
