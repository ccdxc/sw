#!/bin/bash

set -e
set -o pipefail

# This script runs inside a centos container to add venice image to existing venice liveCD
#   The liveCD on bootup will partition and format the sda UNCONDITIONALLY
#   mainly useful for kickstarting and clean installation of the venice installations
#   on fresh new machines

if [ $# -eq 1 -a "$1" == "apulu" ]; then
  echo "building venice install iso for apulu pipeline"
  VENICE_FILE=venice.apulu.tgz
  FW_FILE=naples_fw_venice.tar
  OUT_FILE=pen-install.apulu.iso
else
  echo "building venice install iso for iris pipeline"
  VENICE_FILE=venice.tgz
  FW_FILE=naples_fw.tar
  OUT_FILE=pen-install.iso
fi

if [ ! -f /venice-bin/$VENICE_FILE -o ! -f /nic/$FW_FILE ]; then
  echo "all required files are not found"
  echo "expect venice install file /venice-bin/$VENICE_FILE"
  echo "expect naples firmware file /nic/$FW_FILE"
  exit 1
fi

#expected files
#   /pen/venice.tgz : venice binaries
#   /pen/PEN-VERSION : Version of the build
#   /pen/isolinux.cfg : isolinux.cfg to show the menu on bootup of the iso
#   /pen/venice-os.cfg : Redhat kickstart script to create the ISO
#   /pen/venice-cleaninstall.sh : the script that runs to install ISO content to harddisk
#   /venice-bin/venice.tgz : the venice file
#   /venice-bin/cache is used to cache the packages during creation of iso

#

#start of script

#creation of installer ISO
cd /
mkdir -p /venice-bin/venice-install

# after the iso got created we need to edit the iso. Start by copying the contents of iso to /iso directory
# we mount at a temp location and copy because mounting of iso is done ro
mkdir -p /t
mkdir -p /iso
mount /venice-bin/pen-base.iso /t
cp -a /t/* /iso
umount /t

cd /t
unsquashfs /iso/LiveOS/squashfs.img
mkdir /t2
mount /t/squashfs-root/LiveOS/ext3fs.img /t2

# now update the squashfs as needed
cd /t2
for i in etc/ usr/
do
    cp -fa /pen/$i .
done

curl -sLo /t2/usr/local/bin/jq https://github.com/stedolan/jq/releases/download/jq-1.6/jq-linux64
chmod +x /t2/usr/local/bin/jq

cp /tools/scripts/venice_appl_GrubEntry.sh /t2/usr/local/bin/venice_appl_GrubEntry.sh
chmod +x /t2/usr/local/bin/venice_appl_GrubEntry.sh

systemctl --root=/t2 enable docker
systemctl --root=/t2 enable penservice
systemctl --root=/t2 enable penservice-early

# now put back the squashfs
cd /
dd if=/dev/zero of=/t2/zeros bs=1M || :
sync
rm -f /t2/zeros
umount /t2
rm -f /iso/LiveOS/squashfs.img
mksquashfs /t/squashfs-root /iso/LiveOS/squashfs.img -comp xz

#create a temp iso to be used for pxe bootable image
cd /iso
mkisofs -o /venice-bin/$OUT_FILE \
  -J -r -hide-rr-moved -hide-joliet-trans-tbl -V pen-install \
  -b isolinux/isolinux.bin -c isolinux/boot.cat \
  -no-emul-boot -boot-load-size 4 -boot-info-table -eltorito-alt-boot -e isolinux/efiboot.img -no-emul-boot  \
  -eltorito-alt-boot -e isolinux/macboot.img -no-emul-boot  \
  /iso
/usr/bin/isohybrid -u -m /venice-bin/$OUT_FILE

# creation of PXE bootable installer image
rm -fr /venice-bin/pxe
mkdir -p /venice-bin/pxe
cd /venice-bin/pxe
livecd-iso-to-pxeboot /venice-bin/$OUT_FILE

# PXE bootable minimal image is done. Once booted, it copies 
#  squashfs etc from the below location to the harddisk and makes it bootable
cp /iso/LiveOS/squashfs.img /venice-bin/venice-install/squashfs.img
cp /iso/isolinux/vmlinuz0 /venice-bin/venice-install/vmlinuz0
cp /iso/isolinux/initrd0.img /venice-bin/venice-install/initrd0.img


# now create an installer DVD with venice, naples etc

#copy my copy of isolinux.cfg
cp /pen/isolinux.cfg /iso/isolinux/isolinux.cfg || :
# our own grub.cfg indicating that EFI is not supported
cp /pen/grub-efi.cfg /iso/EFI/BOOT/grub.cfg || :
cp /pen/venice-cleaninstall.sh /iso/LiveOS/venice-cleaninstall.sh || :
# this creates a full-fledged installation dvd with venice and naples
cp /pen/PEN-VERSION /iso/LiveOS/PEN-VERSION || :
cp /venice-bin/$VENICE_FILE /iso/LiveOS/venice.tgz || :
cp /nic/$FW_FILE /iso/LiveOS/naples_fw.tar || :

#create the iso with all our contents and our custom isolinux and grub
cd /iso
mkisofs -o /venice-bin/$OUT_FILE \
  -J -r -hide-rr-moved -hide-joliet-trans-tbl -V pen-install \
  -b isolinux/isolinux.bin -c isolinux/boot.cat \
  -no-emul-boot -boot-load-size 4 -boot-info-table -eltorito-alt-boot -e isolinux/efiboot.img -no-emul-boot  \
  -eltorito-alt-boot -e isolinux/macboot.img -no-emul-boot  \
  /iso
/usr/bin/isohybrid -u -m /venice-bin/$OUT_FILE

chmod -R 777 /venice-bin

exit 0
