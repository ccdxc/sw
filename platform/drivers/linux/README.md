# IONIC Linux Device Drivers

## Build and Install (Kernel and Drivers)

Here at the top, instructions for installing the Linux kernel and drivers on an
existing environment.  Below that, instructions for setting up a new
development environment.  If you are just starting, skip to the third section
and read to the end, before returning to kernel and drivers.

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

```sh
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

## Driver Loading and Basic Usage

These steps are to be run on the host where drivers are installed.  For most
development, this will be QEMU.

### Ethernet Driver

Load the driver and enable the link:

```sh
modprobe ionic
# or: insmod path/to/ionic.ko dyndbg=+pmfl

# use an address in the same subnet as your tap device
ip addr add 172.0.0.2/24 dev enp3s0
ip link set enp3s0 up
```

As an alternative to typing the ip config commands, you can add the interface
to the network configuration.  This step only needs to be followed the first
time, and then the interface can be brought up with `ifup enp3s0`.

```sh
# one time, and remember to replace the address
sudo tee /etc/network/interfaces.d/99-ionic.cfg <<EOF
iface enp3s0 inet static
	address 172.0.0.2/24
EOF

# each time, instead of ip commands
ifup enp3s0
```

Basic connectivity:

The model runs much more slowly than hardware.  To test basic connectivity,
relax the ping timeout.  Even so, the first attempt may fail, due to added
latency of the arp request and response.  After the arp table is updated,
typical ping round trip time through the model is about one second, at the time
of writing.

```sh
ping -c1 -W10 172.0.0.1
```

### RDMA Driver

Load the drivers:

```sh
modprobe ionic_rdma
# or: insmod path/to/ionic_rdma.ko dyndbg=+pmfl

modprobe ib_uverbs
```

Basic connectivity:

```sh
cd path/to/rdma-core/build
bin/ibv_devinfo -v

bin/ibv_rc_pingpong -g3 -r3 -n1 172.0.0.3
# just before that, on the peer (eg, QEMU host running RXE)
# bin/ibv_rc_pingpong -g1 -r3 -n1

# May need to chagnge -gX depending on the output of ibv_devinfo.
```

## Development Environment and Workspace Setup

These are instructions for setting up a new environment for device driver
development.  Some steps may also be used for other development, but these
instructions are intended for device drivers.  After reading this section,
return to the first section to build and install device drivers.

### Set up /local

```sh
# Directory for pensando git repos
mkdir -p /local/you/ws/src/github.com/pensando

# Move bazel (and other) caches off of NFS.
mv ~/.cache /local/you
ln -s /local/you/.cache ~
```

### Clone sw repo

Basic git configuration.

```sh
git config --global user.name 'Your Name'
git config --global user.email 'you@pensando.io'
git config --global push.default current
ssh-keygen
# and upload ~/.ssh/id_rsa.pub to github
```

Clone the sw repo.

```sh
cd /local/you/ws/src/github.com/pensando
git clone git@github.com:pensando/sw.git

# recommended to create a fork on github
cd sw
git remote add you git@github.com:you/sw.git
git config remote.pushdefault you

# for convenience, also add your teammates' forks
git remote add alice git@github.com:alice/sw.git
git remote add bob git@github.com:bob/sw.git

# check out a topic branch for your work
git checkout -b new-gizmo origin/master

# After you commit, push the branch to your fork (this will update an open PR)
git push [-nf]

# When upstream changes, pull or fetch and rebase (from origin/master)
git pull
# or
git fetch && git rebase [-i]

# Once in a while, prune remote tracking branches
git fetch --all --prune

# Because of push.defualt, remote.pushdefault, and checkout origin/master,
#   the repositoriy and branch names don't need to be explicit in the command.
#   The push.default current indicates that the remote branch name should be
#   the same as the current branch name.  The remote.pushdefault says that the
#   remote respository should be your fork for pushing.  Setting the upstream
#   branch in checkout to origin/master sets the default remote and branch name
#   for pull, fetch, and rebase.  All of these can be overridden by specifying
#   different remote and branch names on the command line.
```

### Setup Workspace Environment (pensando-helpers)

It is **important** that users on the same system do not use the same addresses
and port numbers in the configuration that follows.  Please talk to your
teammates when choosing ports and addresses for your configuration.

```sh
# Follow the instructions in the file to edit the config,
#   after copying to your home dir.
cp /local/you/ws/src/github.com/pensando/sw/tools/scripts/pensando-helpers-config.sh ~

# Also, set up your gopath, and import the helpers script in your .bashrc.
cat >> ~/.bashrc <<EOF

export GOPATH="/local/you/ws"
export GOROOT="/usr/local/go"
export PATH=~/bin:$PATH:$GOPATH/bin:$GOROOT/bin

. /local/you/ws/src/github.com/pensando/sw/tools/scripts/pensando-helpers.sh
EOF

# Logout and login again to refresh shell sessions, or reload .bashrc in open
#   sessions.  If the script is changed after loading, use `ph_reload` instead.
```

### Preparing QEMU (ubuntu 16.04 lts xenial cloud image)

Prepare QEMU image:

```sh
cd /local/you

# Base cloud image (will not be modified).
wget https://cloud-images.ubuntu.com/xenial/current/xenial-server-cloudimg-amd64-disk1.img

# Larger image, copy on write, using the cloud image as base image.
qemu-img create -o size=20G -f qcow2 -b xenial-server-cloudimg-amd64-disk1.img sim.img

# Cloud image seed configuration.
cat > sim-seed.txt <<EOF
#cloud-config

ssh_authorized_keys:
  - ssh-rsa [copy your ~/.ssh/id_rsa.pub here]

sudo: ALL=(ALL) NOPASSWD:ALL

packages:
  - build-essential
  - fakeroot
  - libncurses5-dev
  - libssl-dev
  - libelf-dev
  - sshfs
EOF

# Make seed image from configuration
cloud-localds sim-seed.img sim-seed.txt
```

After the first boot (see the section on starting the model, hal, etc), there
is some configuration to be done.  I like to add /local as a sshfs mount point
in qemu.  It is also good at this point to attempt to build rdma-core, and
resolve any dependencies that aren't satisfied.

This is probably the best time to build and install the new kernel, too.  See
instructions at the top.  Modules under development can be installed (or not,
your preference) at a later time.

```sh
sudo tee -a /etc/fstab <<EOF
sshfs#you@host:/local /local fuse defaults,noauto,allow_other 0 0
EOF

# after boot, from an interactive terminal (eg. ssh_qemu):
sudo mount /local
# [type your password]

# make note and install missing build dependencies of rdma-core
cd /local/you/rdma-core
./build.sh

# after configuration, do one clean shutdown, before making copies.
sudo poweroff
```

Prepare second image for rdma peer:

After starting qemu the first time and installing whatever is needed, then I
make a copy of the image for a second instance of qemu, to use as rdma peer.

You can also make a third backup copy of the image, just in case.

```sh
cp sim.img sim-rxe.img
```

### Build the Workspace

```sh
ws pensando/sw
clean_build_all

# or, for incremental build, your mileage may vary:
build_all
# or
make_all
# or
make ws-tools
make pull-assets
make -C nic
make -C platform
make -C platform/src/sim/qemu
make -C platform/src/sim/model_server
```

### Start the model, hal, dol, relay, qemu, qemu_rxe

Recommended to run each sequence of commands in its own terminal.

Model:

```sh
ws pensando/sw/platform
start_model
```

Hal:

```sh
ws pensando/sw/nic
start_hal_classic
```

DoL:

```sh
ws pensando/sw/nic
start_dol
```

Relay:

```sh
ws pensando/sw/nic
start_relay
```

QEMU:

```sh
ws pensando/sw/platform
start_qemu
```

```sh
# serial console
telnet_qemu
```

```sh
# qemu monitor command line interface
monitor_qemu
```

```sh
# ssh session to qemu host (after boot has finished)
ssh_qemu
```

At this point, after starting QEMU for the first time, is where to do some
extra first-boot configuration.  See the section on QEMU setup.  Then, make a
copy of the image file for the rxe host.

QEMU RXE:

```sh
start_qemu_rxe
```

In an ssh session with the rxe host, load the rxe modules and set up the
interface.

```sh
ssh_qemu_rxe

# on QEMU

# first time:
sudo tee /etc/network/interfaces.d/99-rxe.cfg <<EOF
iface enp0s3 inet static
	address 172.0.0.3/24

# each time:
ifup enp0s3
modprobe rdma_rxe
modprobe ib_uverbs
cd /local/you/rdma-core/build
providers/rxe/rxe_cfg.in add enp0s3
```

You made it!  Hopefully...  Go grab snack, maybe a healthy fruit or vegetable.

Now, refer back up to the top to build, install, and load modules, and test
network connectivity through the model.
