#!/bin/sh
if [ -f /tmp/cpldreset ]; then
    REBOOT_REAL=/sbin/reboot-real
    if [ ! -e $REBOOT_REAL ]; then
        mkdir -p /platform/temp/bin
        mkdir -p /platform/temp/lib
        cp $1/bin/cpldapp-real /platform/temp/bin/cpldapp
        cp $1/lib/libpal.so /platform/temp/lib/.
        mv /sbin/reboot /sbin/reboot-real
        cp $1/bin/reboot /sbin/reboot
    fi
fi
