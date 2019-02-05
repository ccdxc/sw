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

mainfwa_partuuid=d4e53be5-7dc1-4199-914c-48edfea92c5e
mainfwb_partuuid=e2fd6d28-3300-4979-8062-b8ab599f3898

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

cur_image=`fwupdate -r`

if [ $cur_image == "mainfwa" ]; then
    new_image="mainfwb"
elif [ $cur_image == "mainfwb" ]; then
    new_image="mainfwa"
else
    echo "Cannot swith_rootfs to altfw from $cur_image"
    echo "switch_rootfs failed!!!"
    exit 1
fi

if [ $new_image == "mainfwa" ]; then
    partuuid=$mainfwa_partuuid
elif [ $new_image == "mainfwb" ]; then
    partuuid=$mainfwb_partuuid
else
    echo "Illegal fw name($1) provided"
    usage
    exit 1
fi

echo "Switching filesystem from $cur_image to $new_image"

mkdir -p /new
mount PARTUUID=$partuuid /new
if [ $? -ne 0 ]; then
    echo "Cannot mount filesystem at PARTUUID: $partuuid"
    echo "Upgrade Failed !!!"
    exit 1
fi

mount -t tmpfs tmpfs /new/mnt
mkdir -p /new/mnt/upper /new/mnt/work
mount -t overlay overlay -o lowerdir=/new,upperdir=/new/mnt/upper,workdir=/new/mnt/work /new/rw
cp -a /var/run/fwupdate.cache /new/rw/var/run/.

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

pivot_root . old
touch /.rw

if [ $? -ne 0 ]; then
    echo "Read-only filesystem under new image"
    unmount_fs
    exit 1
fi

ifconfig bond0 down
rmmod mnet ionic_mnic

kill -QUIT 1

