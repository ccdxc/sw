#!/bin/bash
set -e
set -x

#this script is responsible for mounting of file systems at correct locations
# restore user config and run any plugins to modify the file system as needed
# this is run quite early in the sequence before many other processes are started

# this is a no-op on the installer boot since pen.venice is not there
if [[ -z "`grep pen.venice /proc/cmdline `" ]]
then
    exit 0
fi

# this script is run early on in the venice bootup sequence
# main idea is to mount various partitions and restore user specified config
mkdir -p /config
mkdir -p /data
# Scan for the LVs and wait until we have both LVs available befoe we proceed to mounting them
vgscan --cache
for i in {1..30}; do
    num_lvs=$(lvdisplay | egrep "LV Status.*available" | wc -l)
    if [ "$num_lvs" -eq "2" ]; then
        break
    fi
    sleep 1
done
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

for i in /var/lib/NetworkManager /etc/sysconfig/network-scripts
do
	# first time read from dvd. After that keep bind mounting
	if [[ ! -r ${BINDMNT_ROOT}$i ]]
	then
		cp --parents -a $i ${BINDMNT_ROOT}
	fi

	mount --bind ${BINDMNT_ROOT}$i $i
done



[[ -x /sbin/restorecon ]] && /sbin/restorecon /var/lib/docker
