#!/bin/sh

[ ! -f /sysconfig/config0/no_reboot_on_hostdn ] && /nic/tools/sysreset.sh
