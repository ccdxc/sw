#!/bin/sh

#Reload mnic drivers
/nic/tools/reload_mnic_drv.sh &> /var/log/pensando/reload_mnic_drv.log
[[ $? -ne 0 ]] exit 1

#Add something here for more clean up
#
#
#

#return success at the end
exit 0
