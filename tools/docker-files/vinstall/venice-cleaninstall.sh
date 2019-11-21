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
DEF_SRC=/run/initramfs/live
pen_install_src=${DEF_SRC}
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
    pen.install.src=*)
        pen_install_src="${o#pen.install.src=}"
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


copyLocalFile() {
	rsync --inplace -P "$1" "$2"
}

copyRemoteFile() {
    curl --progress-bar -SL "$1" -o "$2"
}

systemctl stop docker
systemctl disable docker
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
p3_end=${dev_size}

/sbin/parted -s $device u MB mkpart '"Grub"'   fat32 $p1_start $p1_end set 1 bios_grub on
/sbin/parted -s $device u MB mkpart '"Boot"'   ext4 $p2_start $p2_end
/sbin/parted -s $device u MB mkpart '"Config"' ext4 $p3_start $p3_end
# Create LVM for the last two persistent partitions
vg_name=venice_vg
vg_dev=/dev/${vg_name}
lv_name_prefix=${vg_dev}/lv
pvcreate ${device}3
vgcreate ${vg_name} ${device}3
# Config volume is same as the original parition size 3
lvcreate -L ${p3_size}m -n lv3 ${vg_name}
# Data volume fills the rest of the VG
lvcreate -l 100%FREE -n lv4 ${vg_name}

/sbin/udevadm settle
sleep 5

# for each partition
for i in {2..4}
do
	if [ "$i" -le 2 ]; then
	    TGTDEV=${device}${i}
	else
	    TGTDEV=${lv_name_prefix}${i}
	fi
	umount $TGTDEV &> /dev/null || :
	$mkfs -L $(eval "echo \$label$i") $TGTDEV
	eval TGTLABEL${i}="UUID=$(/sbin/blkid -s UUID -o value $TGTDEV)"
done


TGTMNT1=$(mktemp -d /tmp/tgttmp.XXXXXX)
mount -L ${label2} $TGTMNT1
grub2-install --root-directory=${TGTMNT1} ${device}


if [ ${pen_install_src} == ${DEF_SRC} ]
then
    # cd based
    copyLocalFile ${pen_install_src}/LiveOS/PEN-VERSION /tmp/PEN-VERSION

    VERSION=$(grep version: /tmp/PEN-VERSION  | awk '{print $2}')
    TGTDIR=$TGTMNT1/OS-${VERSION}
    mkdir -p $TGTDIR

    copyLocalFile ${pen_install_src}/LiveOS/PEN-VERSION $TGTDIR/PEN-VERSION
    copyLocalFile ${pen_install_src}/LiveOS/squashfs.img $TGTDIR/squashfs.img
    copyLocalFile ${pen_install_src}/isolinux/initrd0.img $TGTDIR/initrd0.img
    copyLocalFile ${pen_install_src}/isolinux/vmlinuz0 $TGTDIR/vmlinuz0
    copyLocalFile ${pen_install_src}/LiveOS/venice.tgz $TGTDIR/venice.tgz
    copyLocalFile ${pen_install_src}/LiveOS/naples_fw.tar $TGTDIR/naples_fw.tar
else
    # http based
    copyRemoteFile ${pen_install_src}/tools/docker-files/vinstall/PEN-VERSION /tmp/PEN-VERSION

    VERSION=$(grep version: /tmp/PEN-VERSION  | awk '{print $2}')
    TGTDIR=$TGTMNT1/OS-${VERSION}
    mkdir -p $TGTDIR

    copyRemoteFile ${pen_install_src}/tools/docker-files/vinstall/PEN-VERSION $TGTDIR/PEN-VERSION
    copyRemoteFile ${pen_install_src}/bin/venice-install/initrd0.img $TGTDIR/initrd0.img
    copyRemoteFile ${pen_install_src}/bin/venice-install/vmlinuz0 $TGTDIR/vmlinuz0
    copyRemoteFile ${pen_install_src}/bin/venice-install/squashfs.img $TGTDIR/squashfs.img
    copyRemoteFile ${pen_install_src}/bin/venice.tgz $TGTDIR/venice.tgz
    copyRemoteFile ${pen_install_src}/nic/naples_fw.tar $TGTDIR/naples_fw.tar
fi


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

menuentry ${VERSION} {
	linux16 /OS-${VERSION}/vmlinuz0 rw rd.fstab=0 root=live:${TGTLABEL2} rd.live.dir=/OS-${VERSION} rd.live.squashimg=squashfs.img console=ttyS0 console=tty0 rd.live.image rd.luks=0 rd.md=0 rd.dm=0  enforcing=0 LANG=en_US.utf8 rd.writable.fsimg=1 pen.venice=OS-${VERSION}/venice.tgz pen.naples=OS-${VERSION}/naples_fw.tar
	initrd16 /OS-${VERSION}/initrd0.img
}

EOF

umount $TGTMNT1


# generate a unique /etc/machine-id file
TGTMNT3=$(mktemp -d /tmp/tgttmp.XXXXXX)
mkdir -p ${TGTMNT3}/copy-fs/
mount -L ${label3} $TGTMNT3
systemd-machine-id-setup --root=${TGTMNT3}/copy-fs/
umount ${TGTMNT3}

fsck -pf ${device}2 || :
fsck -pf ${device}3 || :
fsck -pf ${device}4 || :

echo Succesfully installed the image
echo Eject the installation media or change the boot order and reboot the host

if [ ! -z "`grep pen.live.install /proc/cmdline `" -a ! -z "`grep pen.live.autoreboot /proc/cmdline `" ]
then
    echo "About to reboot in 5 seconds..."
    sleep 5
    /usr/sbin/reboot -f -d -n
fi
