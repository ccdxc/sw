#!/bin/sh

unmount_fs()
{
    echo "Upgrade Failed !!!"

    umount /new/rw
    umount /new/mnt
    umount /new
}

usage()
{
    echo "Usage: ./switch_rootfs.sh altfw"
}

update_fwupgrade_state()
{
    echo $1 >> /var/run/fwupgrade.state
}

mainfwa_partuuid=d4e53be5-7dc1-4199-914c-48edfea92c5e
mainfwb_partuuid=e2fd6d28-3300-4979-8062-b8ab599f3898

#Remove stale /var/run/fwupgrade.state file if present
rm -f /var/run/fwupgrade.state

update_fwupgrade_state "ARG VERIFICATION"

if [ $# -eq 0 ]; then
    echo "No argument provided"
    usage
    exit 1
fi

if [ "$1" == "altfw" ]; then
    echo "Changing the rootfs to altfw"
else
    echo "Invalid arg: $1"
    usage
    exit 1
fi

update_fwupgrade_state "VERIFYING EXISTING IMAGE"

cur_image=`/nic/tools/fwupdate -r`

if [ $cur_image == "mainfwa" ]; then
    new_image="mainfwb"
elif [ $cur_image == "mainfwb" ]; then
    new_image="mainfwa"
else
    echo "Cannot swith_rootfs to altfw from $cur_image"
    echo "switch_rootfs failed!!!"
    exit 1
fi

update_fwupgrade_state "VERIFYING NEW IMAGE NAME"

if [ $new_image == "mainfwa" ]; then
    partuuid=$mainfwa_partuuid
elif [ $new_image == "mainfwb" ]; then
    partuuid=$mainfwb_partuuid
else
    echo "Illegal fw name($1) provided"
    usage
    exit 1
fi

rm -rf /data/pre-upgrade-logs.tar.gz
tar -cvf /data/pre-upgrade-logs.tar.gz /var/log/

echo "Switching filesystem from $cur_image to $new_image"

update_fwupgrade_state "MOUNTING NEW IMAGE PARTITION"

mkdir -p /new
mount PARTUUID=$partuuid /new
if [ $? -ne 0 ]; then
    echo "Cannot mount filesystem at PARTUUID: $partuuid"
    echo "Upgrade Failed !!!"
    exit 1
fi

update_fwupgrade_state "PRESERVING FWUPGRADE STATE TO NEW FILESYSTEM"

mount -t tmpfs tmpfs /new/mnt
mkdir -p /new/mnt/upper /new/mnt/work
mount -t overlay overlay -o lowerdir=/new,upperdir=/new/mnt/upper,workdir=/new/mnt/work /new/rw
cp -a /var/run/fwupdate.cache /new/rw/var/run/.
cp -a /var/run/fwupgrade.state /new/rw/var/run/.

if [ $? -ne 0 ]; then
    echo "Cannot find /var/run/fwupdate.cache !!!"
    unmount_fs
    exit 1
fi

#mount the new disk at /ro under new rootfs so we can figure out which mmc partition we are in once we do pivot_root
mount --bind --no-mtab /new /new/rw/ro

umount /dev/shm
umount /run
umount /tmp

mount --move --no-mtab /dev /new/rw/dev
mount --move --no-mtab /proc /new/rw/proc

cd /new/rw
mkdir -p old

echo "PERFORMING PIVOT_ROOT TO NEW FS" >> /new/rw/var/run/fwupgrade.state

pivot_root . old

update_fwupgrade_state "TEST WHETHER NEW FS IS WRITEABLE OR NOT"

touch /.rw

if [ $? -ne 0 ]; then
    echo "Read-only filesystem under new image"
    unmount_fs
    exit 1
fi

update_fwupgrade_state "REMOVING MNIC DRIVERS FROM KERNEL"

ifconfig bond0 down
rmmod mnet mnet_uio_pdrv_genirq ionic_mnic

update_fwupgrade_state "KILLING INIT..."

kill -QUIT 1

