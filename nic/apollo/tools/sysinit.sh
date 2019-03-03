#!/bin/sh

export NIC_DIR='/nic'
export LD_LIBRARY_PATH=$NIC_DIR/lib
ulimit -c unlimited
cd /
# start pciemgrd
/nic/bin/pciemgrd-gold &
# start memtun
/nic/bin/memtun &

# if agent exists, start it
if [[ -f $NIC_DIR/tools/start-agent.sh ]]; then
    echo "Launching agent ..."
    $NIC_DIR/tools/start-agent.sh
fi

echo "Launched all applications ..."
