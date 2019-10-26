#!/bin/sh

export NIC_DIR='/nic'
export LD_LIBRARY_PATH=$NIC_DIR/lib
ulimit -c unlimited
cd /
ifconfig lo up

# load kernel modules for mnics
insmod /nic/bin/ionic_mnic.ko &> /var/log/pensando/ionic_mnic_load.log
[[ $? -ne 0 ]] && echo "Aborting sysinit, failed to load mnic driver!" && exit 1

insmod /nic/bin/mnet_uio_pdrv_genirq.ko &> /var/log/pensando/mnet_uio_pdrv_genirq_load.log
[[ $? -ne 0 ]] && echo "Aborting sysinit, failed to load mnet_uio_pdrv_genirq driver!" && exit 1

insmod /nic/bin/mnet.ko &> /var/log/pensando/mnet_load.log
[[ $? -ne 0 ]] && echo "Aborting sysinit, failed to load mnet driver!" && exit 1

# start agent
if [[ -f $NIC_DIR/tools/start-agent.sh ]]; then
    echo "Launching agent ..."
    $NIC_DIR/tools/start-agent.sh
    [[ $? -ne 0 ]] && echo "Aborting sysinit, failed to start agent!" && exit 1
fi

# start cronjobs
nice crond -c /nic/conf/apollo/crontabs

echo "System initialization done ..."
