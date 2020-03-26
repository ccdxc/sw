#!/bin/sh

#timeout if bond0 doesn't get created in 60 seconds
exec env TMPDIR=/data NAPLES_PIPELINE=apollo /naples/nic/bin/nmd -oob-interface eth1 -inb-interface eth1
