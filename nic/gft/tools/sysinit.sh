#!/bin/sh

export NIC_DIR='/nic'
export LD_LIBRARY_PATH=$NIC_DIR/lib
ulimit -c unlimited
cd /
ifconfig lo up
# start pciemgrd
/nic/bin/pciemgrd-gold &
# start memtun
/nic/bin/memtun &

# load kernel modules for mnics
#insmod /nic/bin/ionic_mnic.ko &> /var/log/pensando/ionic_mnic_load.log
#[[ $? -ne 0 ]] && echo "Aborting sysinit, failed to load mnic driver!" && exit 1

#insmod /nic/bin/mnet_uio_pdrv_genirq.ko &> /var/log/pensando/mnet_uio_pdrv_genirq_load.log
#[[ $? -ne 0 ]] && echo "Aborting sysinit, failed to load mnet_uio_pdrv_genirq driver!" && exit 1

#insmod /nic/bin/mnet.ko &> /var/log/pensando/mnet_load.log
#[[ $? -ne 0 ]] && echo "Aborting sysinit, failed to load mnet driver!" && exit 1

echo "System initialization done ..."
