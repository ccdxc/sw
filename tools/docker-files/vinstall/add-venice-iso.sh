#!/bin/bash

set -e
set -o pipefail

# This script runs inside a centos container to add venice image to existing venice liveCD
#   The liveCD on bootup will partition and format the sda UNCONDITIONALLY
#   mainly useful for kickstarting and clean installation of the venice installations
#   on fresh new machines


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
rm -fr /venice-bin/pxe
mkdir -p /venice-bin/pxe
cd /venice-bin/pxe
livecd-iso-to-pxeboot /venice-bin/pen-install.iso

cd /

# after the iso got created we need to edit the iso. Start by copying the contents of iso to /iso directory
# we mount at a temp location and copy because mounting of iso is done ro
mkdir -p /t
mkdir -p /iso
mount /venice-bin/pen-install.iso /t
cp -a /t/* /iso
umount /t

mkdir -p /venice-bin/venice-install
cp /iso/LiveOS/squashfs.img /venice-bin/venice-install/squashfs.img
cp /iso/isolinux/vmlinuz0 /venice-bin/venice-install/vmlinuz0
cp /iso/isolinux/initrd0.img /venice-bin/venice-install/initrd0.img

#copy my copy of isolinux.cfg
cp /pen/isolinux.cfg /iso/isolinux/isolinux.cfg || :
# our own grub.cfg indicating that EFI is not supported
cp /pen/grub-efi.cfg /iso/EFI/BOOT/grub.cfg || :

#cp /pen/PEN-VERSION /iso/LiveOS/PEN-VERSION || :
#cp /pen/venice-cleaninstall.sh /iso/LiveOS/venice-cleaninstall.sh || :

#finally create the iso back with our custom isolinux and grub
cd /iso
mkisofs -o /venice-bin/pen-install.iso \
  -J -r -hide-rr-moved -hide-joliet-trans-tbl -V pen-install \
  -b isolinux/isolinux.bin -c isolinux/boot.cat \
  -no-emul-boot -boot-load-size 4 -boot-info-table -eltorito-alt-boot -e isolinux/efiboot.img -no-emul-boot  \
  -eltorito-alt-boot -e isolinux/macboot.img -no-emul-boot  \
  /iso
/usr/bin/isohybrid -u -m /venice-bin/pen-install.iso

chmod -R 777 /venice-bin

exit 0
