#!/bin/sh
ifconfig bond0 down
kill -9 $(pidof dhclient)
exec env TMPDIR=/data /nic/bin/nmd
