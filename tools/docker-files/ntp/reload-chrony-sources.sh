#!/bin/sh
set -x
date

for i in $(chronyc -c sources | cut -d, -f3)
do
    chronyc delete $i
done

for i in $(grep server /etc/pensando/pen-ntp/chrony.conf  | grep iburst | awk '{print $2}')
do
    chronyc add server $i iburst
done

date

# Wait for chronyc to synchronize to the new sources,
# give up after 3 attempts, 10s apart (30s max)
# If it fails to sync now, it will still try to sync
# in the background but adjustment will be slow
chronyc waitsync 3

# force local clock sync to the new sources
chronyc makestep

date
set +x
exit 0
