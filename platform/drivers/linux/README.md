# IONIC Linux Device Drivers

## Build and Install

### Linux Kernel

Build and install the kernel (or obtain the configured kernel) for the
environment where drivers will be installed.

I use sshfs to mount the system where the sources are located, and generate the
kernel config and install the modules and kernel on QEMU.  I then build and
install out-of-tree modules using the same procedure.  I actually build the
kernel on srv14 after it is configured, before returning to the QEMU
environment to intall the modules and kernel.

```sh
# on build host
git clone https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
cd linux

# on QEMU
cp /lib/modules/`uname -r`/build/.config .
make olddefconfig
make localmodconfig

# on build host, eg srv14
make -j8

# on QEMU
sudo make modules_install
sudo make install
sudo reboot
# ...
uname -r
# uname -r should be the new kernel version
```

### Ionic Drivers

Build and install the kernel space drivers.

```
# on build host
cd /path/to/sw/platform/drivers/linux
make -j8 KSRC=/path/to/linux

# on QEMU
sudo make modules_install

# Recommended (on QEMU):
sudo tee /etc/modprobe.d/ionic.conf <<EOF
blacklist ionic
options ionic dyndbg=+pmfl
EOF
sudo tee /etc/modprobe.d/ionic_rdma.conf <<EOF
blacklist ionic_rdma
options ionic_rdma dyndbg=+pmfl
EOF
```

To install the RDMA user space drivers, see [README][rdma-usersp].

[rdma-usersp]: rdma/lib/ionic/README.md
