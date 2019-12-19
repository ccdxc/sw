#!/bin/sh

#timeout if bond0 doesn't get created in 60 seconds
read -t 60 <> /tmp/bond0_fifo
rm /tmp/bond0_fifo
ifconfig bond0 down
kill -9 $(pidof dhclient)
exec env TMPDIR=/data /nic/bin/nmd
