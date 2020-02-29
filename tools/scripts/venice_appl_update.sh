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
    VER=0.15.0_last-built
    curl -O http://pxe/builds/hourly/${VER}/VERSION
    VER=$(cat VERSION) # something like 0.9.0-79 instead of aliases live 0.9.0_last-built
    rm VERSION
fi

if [ -z "$IP" ]
then
    echo unknown ip address
    echo  usage: $0 -i ipAddr [-v version] [-c]
    exit 1
fi


SSH="ssh -oUserKnownHostsFile=/dev/null -o StrictHostKeyChecking=false"
SCP="scp -oUserKnownHostsFile=/dev/null -o StrictHostKeyChecking=false"


if [ ! -z "$CLEAN" ]
then
sshpass -p${PASS} ${SSH} root@${IP} "cd /tmp; \
curl -O http://pxe/builds/hourly/${VER}/sw-iris/sw/tools/scripts/INSTALL.sh ;
chmod +x ./INSTALL.sh ; \
./INSTALL.sh --clean-only; "
fi

sshpass -p${PASS} ${SSH} root@${IP} "rm -fr /data/installtmp ; mkdir -p /data/installtmp && cd /data/installtmp; \
curl -O http://pxe/builds/hourly/${VER}/bundle/bundle.tar; \
tar xvf bundle.tar ; \
echo rm bundle.tar ; \
curl -o /tmp/venice_appl_GrubEntry.sh http://pxe/kickstart/veniceTempInstall/venice_appl_GrubEntry.sh ; \
chmod +x /tmp/venice_appl_GrubEntry.sh ; \
/tmp/venice_appl_GrubEntry.sh -p /data/installtmp && /tmp/venice_appl_GrubEntry.sh -u /data/installtmp && echo Successfull installed OS version ${VER}. Reboot to take effect; "
