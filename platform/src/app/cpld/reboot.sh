#!/bin/sh

# sync the disks, and then check if a cpldreset is pending
# if a cpld reboot is pending use cpld reboot else reboot cleanly.
CPLD_UPGRADE=/tmp/cpldreset
if [ -e $CPLD_UPGRADE ]; then
    export LD_LIBRARY_PATH=/platform/temp/lib
    exec /platform/temp/bin/cpldapp -reload
fi
watchdog -T 10 /dev/watchdog
reboot "$@"
