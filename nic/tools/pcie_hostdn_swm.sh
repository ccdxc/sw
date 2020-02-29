#!/bin/sh

FW_COMPLETE=/tmp/fwupdate_complete
PROFILE_CHANGE=/tmp/profile_change

# Reboot if there is a pending fwupdate or profile change.
if [ -f "$FW_COMPLETE" -o -f "$PROFILE_CHANGE" ]; then
    /nic/tools/sysreset.sh
fi
