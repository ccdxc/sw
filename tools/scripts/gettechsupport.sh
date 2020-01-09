#!/bin/bash

#use this script to collect techsupport everything else fails
#should be present under /usr/pensando/bin
#run as root from the docker/vm/appliance

journalctl -a > /var/log/pensando/journalctl.log
uptime > /var/log/pensando/uptime
tar -zcf /data/techsupport.tgz /var/log/pensando/* /var/log/dmesg* /etc/pensando/ /var/lib/pensando/pki/ /var/lib/pensando/events/
echo "techsupport is created as /data/techsupport.tgz"


