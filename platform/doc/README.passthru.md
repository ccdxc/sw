# Development with Virtual Machines, PCI Passthrough

## Common

Enter BIOS settings to configure Intel VT-d.

For Cisco UCS Servers:

1. Reboot
2. `F2` Enter Setup at BIOS splash screen
3. Navigate to Intel VT for Directed I/O (VT-d)
   1. Advanced
   2. Socket Configuration
   3. IIO Configuration
   4. Intel VT for Directed I/O (VT-d)
4. Configure Intel VT-d:
   - Intel VT for Directed I/O (VT-d) `[Enable]`
   - Interrupt Remapping	`[Enable]`
   - PassThrough DMA `[Enable]`
5. Unsure: these other settings work for me, not sure if needed
   - ATS `[Enable]`
   - Posed Interrupt `[Enable]`
   - Coherency Support (Non-Isoch) `[Disable]`
6. `F10` Save & Reset System

## Linux Host

Edit kerenel command line in grub configuration.

1. Edit /etc/default/grub, uncomment and/or modify variable:
   - `GRUB_CMDLINE_LINUX="intel_iommu=on"`
2. Rebuild grub menus, and reboot:
   1. `update-grub2`
   2. `reboot`
3. Verify settings have taken effect after reboot:
   1. `grep intel_iommu /proc/cmdline`
      - `BOOT_IMAGE=/boot/vmlinuz-4.15.0-39-generic root=UUID=1644f502-eaee-4406-97e2-6bd881206597 ro intel_iommu=on quiet splash vt.handoff=1`
   2. `dmesg | grep DMAR`
      - `[    1.410472] DMAR: Intel(R) Virtualization Technology for Directed I/O`
   3. `ls /sys/kernel/iommu_groups`
      - not empty

Configure one or more PCI devices for passthrough.

1. Unbind devices from current driver (if any).
   1. `lspci -d 1dd8:`
   2. `echo 'BB:DD.ff' > '/sys/bus/pci/devices/0000:BB:DD.ff/driver/unbind'`
2. Associate devices with PCI passthrough driver.
   1. `modprobe vfio-pci`
   2. Bind devices by PCI ID:
      - `echo '1dd8 1002' > /sys/bus/pci/drivers/vfio-pci/new_id`
   3. Or, bind specific devices by PCI BB:DD.ff:
      - `echo 'BB:DD.ff' > /sys/bus/pci/drivers/vfio-pci/bind`

Specify host device to pass through to virtual machine (qemu).

`qemu-system-x86_64 -machine accel=kvm -device vfio-pci,host=BB:DD.ff [...]`

Eg:

```sh
#!/bin/bash
qemu-system-x86_64 \
	-machine accel=kvm \
	-smp cpus=4 \
	-m size=16384 \
	-nographic -vnc :42 \
	-netdev user,id=net0,hostfwd=tcp::42-:22 \
	-device e1000,netdev=net0 \
	-device vfio-pci,host=b6:00.0 \
	/root/vm/freebsd-allenbh-1.qcow2
```

Warnings such as below seem to be harmless:

```
qemu-system-x86_64: warning: host doesn't support requested feature: CPUID.80000001H:ECX.svm [bit 2]
```

## FreeBSD Host

TODO: instructions for bhyve

## Linux Virtual Machine

TODO: instructions for packer image

## FreeBSD Virtual Machine

TODO: replace with instructions for packer image

### Quick-Start:

Copy the prepared standalone image from
`root@lab-bm20:vm/freebsd-prepared.qcow2`.

Skip image and first-boot preparation.

Login with root/docker.

### Prepare FreeBSD VM Image:

Image preparation:

```sh
mkdir /root/vm && cd /root/vm

# wget https://download.freebsd.org/ftp/releases/VM-IMAGES/11.2-RELEASE/aarch64/Latest/FreeBSD-11.2-RELEASE-arm64-aarch64.qcow2.xz
# xz -d FreeBSD-11.2-RELEASE-arm64-aarch64.qcow2.xz

# instead of hitting download.freebsd.org, copy from another server here (and skip decompressing the xz):
scp root@lab-bm20:/vm/FreeBSD-11.2-RELEASE-arm64-aarch64.qcow2 .

# prepare image using base image:
qemu-img create -f qcow2 -F qcow2 -b FreeBSD-11.2-RELEASE-amd64.qcow2 [NewImageName].qcow2 40G

# Or prepare image standalone:
cp FreeBSD-11.2-RELEASE-amd64.qcow2 [NewImageName].qcow2
qemu-img resize [NewImageName].qcow2 40G
```

First-boot preparation:

Connect to vm via vnc at `host-name:vnc-port`, where host is the host machine
and vnc port is `N` in `qemu-system-x86_64 -vnc N`.

Login on the console as root, no password.

```sh
# Configure root password
passwd root

# Configure/enable sshd with root password login (insecure!)
echo 'PermitRootLogin yes' >> /etc/ssh/sshd_config
echo 'sshd_enable="yes"' >> /etc/rc.conf
service sshd start

# From outside the vm, it should be possible to ssh the vm
ssh -p N root@host-name

#
# Prequisites for Pensando software / rdma drivers
#

pkg install git

# clone freebsd into /usr/src (lab-bm20 instead of github.com)
git clone root@lab-bm20.pensando.io:freebsd -o lab-bm20 -b releng/11.2 /usr/src

echo 'WITH_OFED="yes"' >> /etc/src.conf
echo 'options         OFED' >> /usr/src/sys/amd64/conf/GENERIC
echo 'options         COMPAT_LINUXKPI' >> /usr/src/sys/amd64/conf/GENERIC
make -C /usr/src buildkernel buildworld installkernel installworld
reboot
```
