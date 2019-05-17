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
mkdir -p /venice-bin/venice-install

# after the iso got created we need to edit the iso. Start by copying the contents of iso to /iso directory
# we mount at a temp location and copy because mounting of iso is done ro
mkdir -p /t
mkdir -p /iso
mount /venice-bin/pen-install.iso /t
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
curl -sLo /pen/usr/local/bin/jq https://github.com/stedolan/jq/releases/download/jq-1.6/jq-linux64
chmod +x /pen/usr/local/bin/jq

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

#also keep a copy so that we can do pxe-install
cp /iso/LiveOS/squashfs.img /venice-bin/venice-install/squashfs.img
cp /iso/isolinux/vmlinuz0 /venice-bin/venice-install/vmlinuz0
cp /iso/isolinux/initrd0.img /venice-bin/venice-install/initrd0.img


#copy my copy of isolinux.cfg
cp /pen/isolinux.cfg /iso/isolinux/isolinux.cfg || :
# our own grub.cfg indicating that EFI is not supported
cp /pen/grub-efi.cfg /iso/EFI/BOOT/grub.cfg || :

# this creates a full-fledged installation dvd with venice and naples
cp /pen/PEN-VERSION /iso/LiveOS/PEN-VERSION || :
cp /pen/venice-cleaninstall.sh /iso/LiveOS/venice-cleaninstall.sh || :
cp /venice-bin/venice.tgz /iso/LiveOS/venice.tgz || :
cp /nic/naples_fw.tar /iso/LiveOS/naples_fw.tar || :

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
