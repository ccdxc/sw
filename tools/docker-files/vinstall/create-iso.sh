#!/bin/bash

set -e
set -o pipefail

# This script runs inside a centos container to create the liveCD
#   The liveCD on bootup will partition and format the sda UNCONDITIONALLY
#   mainly useful for kickstarting and clean installation of the venice installations
#   on fresh new machines. This script creates base iso which is built once in a while
# on every pensando build, the base iso is taken, added pensando specific scripts to create
#  pensando iso as well as pxe bootable images


#expected files
#   /pen/isolinux.cfg : isolinux.cfg to show the menu on bootup of the iso
#   /pen/venice-os.cfg : Redhat kickstart script to create the ISO
#   /venice-bin/cache is used to cache the packages during creation of iso


#start of script
cd /
yum install -y livecd-tools rsync dosfstools

#our custom modules
sed -i -e 's/=ata/=ata", "mptspi", "vmw_pvscsi", "megaraid", "megaraid_sas", "hpilo", "hpwdt", "serio_raw", "smartpqi", "tg3", "xfs/' /usr/lib/python2.7/site-packages/imgcreate/live.py

livecd-creator -d --verbose --config=/pen/venice-os.cfg --cache=/venice-bin/cache --fslabel=pen-install

# after the iso got created we need to edit the iso. Start by copying the contents of iso to /iso directory
# we mount at a temp location and copy because mounting of iso is done ro
mkdir -p /t
mkdir -p /iso
mount /pen-install.iso /t
cp -a /t/* /iso
umount /t

#copy my copy of isolinux.cfg
cp /pen/isolinux.cfg /iso/isolinux/isolinux.cfg || :
# our own grub.cfg indicating that EFI is not supported
cp /pen/grub-efi.cfg /iso/EFI/BOOT/grub.cfg || :

#finally create the iso back with our custom isolinux and grub
cd /iso
mkisofs -o /venice-bin/pen-base.iso \
  -J -r -hide-rr-moved -hide-joliet-trans-tbl -V pen-install \
  -b isolinux/isolinux.bin -c isolinux/boot.cat \
  -no-emul-boot -boot-load-size 4 -boot-info-table -eltorito-alt-boot -e isolinux/efiboot.img -no-emul-boot  \
  -eltorito-alt-boot -e isolinux/macboot.img -no-emul-boot  \
  /iso
/usr/bin/isohybrid -u -m /venice-bin/pen-base.iso

exit 0
