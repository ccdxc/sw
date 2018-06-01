#!/bin/sh

for i in $(chronyc -c sources | cut -d, -f3)
do
    chronyc delete $i
done

for i in $(grep server /etc/pensando/pen-ntp/chrony.conf  | grep iburst | awk '{print $2}')
do
    chronyc add server $i iburst
done
exit 0
