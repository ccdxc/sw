#!/bin/sh

#reload all the drivers
echo "Reloading mnic drivers"
rmmod mnet mnet_uio_pdrv_genirq ionic_mnic
insmod /platform/drivers/ionic_mnic.ko
[[ $? -ne 0 ]] && echo "Aborting Sysinit - Unable to load mnic driver!" && exit 1
insmod /platform/drivers/mnet_uio_pdrv_genirq.ko
[[ $? -ne 0 ]] && echo "Aborting Sysinit - Unable to load mnet_uio_pdrv_genirq driver!" && exit 1
insmod /platform/drivers/mnet.ko
[[ $? -ne 0 ]] && echo "Aborting Sysinit - Unable to load mnet driver!" && exit 1
exit 0
