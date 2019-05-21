#!/bin/bash
set -e
set -x

if [[ -z "`grep pen.venice /proc/cmdline `" ]]
then
    exit 0
fi

mkdir -p /config
mkdir -p /data
mount -L PENDATA /data
mount -L PENCFG /config
mkdir -p /data/docker
mkdir -p /data/lib/etcd
mkdir -p /data/var/log
mkdir -p /config/etc
mkdir -p /config/etcd
mkdir -p /var/lib/docker
mkdir -p /var/lib/pensando
mkdir -p /var/log
mkdir -p /etc/pensando

mount --bind /data/var/log /var/log
mount --bind /run/initramfs/live/boot/ /boot
mount --bind /data/docker /var/lib/docker
mount --bind /data/lib /var/lib/pensando
mount --bind /config/etc /etc/pensando
mount --bind /config/etcd /var/lib/pensando/etcd # make sure this is after /var/lib/pensando above


# All scripts are run from here
if [[ -d /config/rc.d ]]
then
	for i in /config/rc.d/*
	do
		$i
	done
fi

cd /
mkdir -p /config/copy-fs
cd /config/copy-fs
cp -a . /

cd /

BINDMNT_ROOT=/config/bind-mnt
mkdir -p ${BINDMNT_ROOT}

for i in /var/lib/NetworkManager
do
	# first time read from dvd. After that keep bind mounting
	if [[ ! -r ${BINDMNT_ROOT}$i ]]
	then
		cp --parents -a $i ${BINDMNT_ROOT}
	fi

	mount --bind ${BINDMNT_ROOT}$i $i
done



[[ -x /sbin/restorecon ]] && /sbin/restorecon /var/lib/docker
