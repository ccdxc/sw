#!/bin/bash
set -e
set -x

if [ -z "`grep pen.venice /proc/cmdline `" ]
then
    exit 0
fi

# read some variables out of /proc/cmdline
for o in `cat /proc/cmdline` ; do
    case $o in
    pen.naples=*)
        pen_naples="${o#pen.naples=}"
        ;;
    pen.venice=*)
        pen_venice="${o#pen.venice=}"
        ;;
    esac
done


if [ ! -z "`grep pen.naples /proc/cmdline `" ]
then
    mkdir -p /var/lib/pensando/images
    cp /run/initramfs/live/${pen_naples} /var/lib/pensando/images/naples_fw.tar
fi

if [ ! -z "`grep pen.venice /proc/cmdline `" ]
then
    systemctl start docker.service # enable docker now
    mkdir -p /tmp/venice
    cd /tmp/venice
    tar zxvf  /run/initramfs/live/${pen_venice}
    if [ -f INSTALL.sh ]
    then
        ./INSTALL.sh || :
    fi
    cd /
    rm -fr /tmp/venice
fi


