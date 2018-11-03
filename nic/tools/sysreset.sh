#!/bin/sh

# sync the disks, start a contingency watchdog, and then give the system
# a chance to reboot cleanly.
sync
watchdog -T 10 /dev/watchdog
reboot
