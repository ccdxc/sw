#!/bin/bash

#set -x
set -e

#install a new image from build directory
#
# usage: venice_appl_update.sh -i ipAddr [-v version] [ -p password ]

PASS=centos

while getopts ":v:i:c" arg; do
  case $arg in
    v) VER=$OPTARG;;
    i) IP=$OPTARG;;
    p) PASS=$OPTARG;;
    c) CLEAN=true;;
    \? ) echo "usage: $0 -i veniceHost [ -v version ] [ -p password ] [-c]" ; exit 0;;
  esac
done


if [ -z "$VER" ]
then
    VER=0.10.0_last-built
fi

if [ -z "$IP" ]
then
    echo unknown ip address
    echo  usage: $0 -i ipAddr [-v version] [-c]
    exit 1
fi


SSH="ssh -oUserKnownHostsFile=/dev/null -o StrictHostKeyChecking=false"
SCP="scp -oUserKnownHostsFile=/dev/null -o StrictHostKeyChecking=false"

curl -O http://pxe/builds/hourly/${VER}/VERSION
VER=$(cat VERSION) # something like 0.9.0-79 instead of aliases live 0.9.0_last-built
rm VERSION

if [ ! -z "$CLEAN" ]
then
sshpass -p${PASS} ${SSH} root@${IP} "cd /tmp; \
curl -O http://pxe/builds/hourly/${VER}/src/github.com/pensando/sw/tools/scripts/INSTALL.sh ;
chmod +x ./INSTALL.sh ; \
./INSTALL.sh --clean-only; "
fi

sshpass -p${PASS} ${SSH} root@${IP} "mkdir -p /run/initramfs/live/OS-${VER}; cd /run/initramfs/live/OS-${VER} ; \
curl -O http://pxe/builds/hourly/${VER}/src/github.com/pensando/sw/bin/venice-install/squashfs.img ; \
curl -O http://pxe/builds/hourly/${VER}/src/github.com/pensando/sw/bin/venice-install/vmlinuz0; \
curl -O http://pxe/builds/hourly/${VER}/src/github.com/pensando/sw/bin/venice-install/initrd0.img; \
curl -O http://pxe/builds/hourly/${VER}/src/github.com/pensando/sw/bin/venice.tgz; \
curl -O http://pxe/builds/hourly/${VER}/src/github.com/pensando/sw/nic/naples_fw.tar; \
curl -O http://pxe/builds/hourly/${VER}/src/github.com/pensando/sw/tools/docker-files/vinstall/PEN-VERSION ;\
cd /tmp; \
curl -O http://pxe/kickstart/veniceTempInstall/venice_appl_GrubEntry.sh ; \
chmod +x venice_appl_GrubEntry.sh ; \
./venice_appl_GrubEntry.sh -a -v ${VER} && echo Successfull installed OS version ${VER}. Reboot to take effect; "
