#!/bin/bash
#set -x
set -e
set -o pipefail

# after installation DVD is booted up, this script is run to format
#   the harddisk and copy the OS and Venice contents to hardisk

device=
mkfs=/sbin/mkfs.ext4
label1=PENGRUB
label2=PENBOOT
label3=PENCFG
label4=PENDATA
SRCMNT=/run/initramfs/live
debug=0

if [ -z "$(grep pen.live.install /proc/cmdline )" -a $debug -eq 0 ]
then
    echo Not running from install media
    exit 1
fi

# if pen.live.device=sde is seen in command-line we will install to /dev/sde instead of /dev/sda
for o in `cat /proc/cmdline` ; do
    case $o in
    pen.live.device=*)
        device="${o#pen.live.device=}"
        ;;
    esac
done

if [ -z "$device" ]
then
    # first disk device in the system
    device=$(lsblk -Sdno NAME,TYPE | grep disk | head -n1 | awk '{print $1}')
fi

if [ -z "$device" ]
then
    echo No Installable device found
    exit 2
fi

device=/dev/${device}
echo Installing to ${device}

# version of the pensando-base image
VERSION=$(grep version: /run/initramfs/live/LiveOS/PEN-VERSION  | awk '{print $2}')
if [ -z "$VERSION" ]
then
    VERSION=pver1
fi

copyFile() {
	rsync --inplace -P "$1" "$2"
}

# this will delete everything from /dev/sda
/usr/sbin/wipefs -a $device
LC_ALL=C /sbin/parted --script $device mklabel gpt
partinfo=$(LC_ALL=C /sbin/parted --script -m $device "unit MB print" |grep ^$device:)
dev_size=$(echo $partinfo |cut -d : -f 2 |sed -e 's/MB$//')

# without this parted keeps complaining about alignment size
dev_size=$(($dev_size - 3))

if [ $dev_size -le $((8 * 1024 - 10)) ]; then
	p1_size=30
	p2_size=2000
	p3_size=2000
elif [ $dev_size -le 20000 ]; then
	p1_size=30
	p2_size=3000
	p3_size=3000
else
	p1_size=30
    p2_size=$((10*1024))
	p3_size=$((20*1024))
fi

p1_start=1
p1_end=$(($p1_start + $p1_size - 1))
p2_start=$(($p1_end + 1))
p2_end=$(($p2_start + $p2_size - 1))
p3_start=$(($p2_end + 1))
p3_end=$(($p3_start + $p3_size - 1))
p4_start=$(($p3_end + 1))
p4_end=${dev_size}

/sbin/parted -s $device u MB mkpart '"Grub"'   fat32 $p1_start $p1_end set 1 bios_grub on
/sbin/parted -s $device u MB mkpart '"Boot"'   ext4 $p2_start $p2_end
/sbin/parted -s $device u MB mkpart '"Config"' ext4 $p3_start $p3_end
/sbin/parted -s $device u MB mkpart '"Data"'   ext4 $p4_start $p4_end
/sbin/udevadm settle
sleep 5

# for each partition
for i in {2..4}
do
	TGTDEV=${device}${i}
	umount $TGTDEV &> /dev/null || :
	$mkfs -L $(eval "echo \$label$i") $TGTDEV
	eval TGTLABEL${i}="UUID=$(/sbin/blkid -s UUID -o value $TGTDEV)"
done


TGTMNT1=$(mktemp -d /tmp/tgttmp.XXXXXX)
mount -L ${label2} $TGTMNT1
grub2-install --root-directory=${TGTMNT1} ${device}

TGTDIR=$TGTMNT1/OS-${VERSION}
mkdir -p $TGTDIR
copyFile $SRCMNT/LiveOS/PEN-VERSION $TGTDIR/PEN-VERSION
copyFile $SRCMNT/LiveOS/squashfs.img $TGTDIR/squashfs.img
copyFile $SRCMNT/isolinux/initrd0.img $TGTDIR/initrd0.img
copyFile $SRCMNT/isolinux/vmlinuz0 $TGTDIR/vmlinuz0
copyFile $SRCMNT/LiveOS/venice.tgz $TGTDIR/venice.tgz


cat >${TGTMNT1}/boot/grub2/grub.cfg <<EOF
set timeout=10
set default=0

insmod gzio
insmod xzio
insmod part_msdos
insmod squash4
insmod iso9660
insmod ext2
insmod btrfs
insmod regexp

menuentry pen${VERSION} {
	linux16 /OS-${VERSION}/vmlinuz0 rw rd.fstab=0 root=live:${TGTLABEL2} rd.live.dir=/OS-${VERSION} rd.live.squashimg=squashfs.img console=ttyS0 console=tty0 rd.live.image rd.luks=0 rd.md=0 rd.dm=0  enforcing=0 LANG=en_US.utf8 rd.writable.fsimg=1 pen.venice=OS-${VERSION}/venice.tgz
	initrd16 /OS-${VERSION}/initrd0.img
}

EOF

umount $TGTMNT1

fsck -f ${device}2 || :
fsck -f ${device}3 || :
fsck -f ${device}4 || :

echo Succesfully installed the image
echo Eject the installation media or change the boot order and reboot the host

if [ ! -z "`grep pen.live.install /proc/cmdline `" -a ! -z "`grep pen.live.autoreboot /proc/cmdline `" ]
then
    echo "About to reboot in 5 seconds..."
    sleep 5
    /usr/sbin/reboot -f -d -n
fi
